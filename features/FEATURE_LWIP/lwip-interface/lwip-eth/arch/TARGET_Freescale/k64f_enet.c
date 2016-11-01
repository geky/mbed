#include "k64f_enet.h"
#include "k64f_phy.h"

#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/tcpip.h"
#include "lwip/ethip6.h"
#include "lwip/igmp.h"
#include "lwip/mld6.h"
#include "netif/etharp.h"
#include "netif/ppp/pppoe.h"

#include "eth_arch.h"
#include "sys_arch.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "equeue.h"


/* Internal state for k64f enet driver */
static struct k64f_enet {
  struct netif *netif;
  enet_handle_t handle;
  equeue_t queue;

  uint8_t tx_produce_index;
  uint8_t tx_consume_index;
  uint8_t tx_buff_count;
  uint8_t rx_index;

  ENET_ALIGNED(ENET_BUFF_ALIGNMENT)
      volatile enet_tx_bd_struct_t tx_desc_buff[ENET_TX_RING_LEN];
  ENET_ALIGNED(ENET_BUFF_ALIGNMENT)
      volatile enet_rx_bd_struct_t rx_desc_buff[ENET_RX_RING_LEN];

  struct pbuf *tx_buff[ENET_TX_RING_LEN];
  struct pbuf *rx_buff[ENET_RX_RING_LEN];
  uint8_t *rx_ptr_buff[ENET_RX_RING_LEN];

  uint8_t event_buff[EQUEUE_EVENT_SIZE*ENET_EVENT_COUNT];
} k64f_enet;


/** \brief  Queue a new RX buffer on the EMAC interface
 *
 *  \param[in]      buf  Pointer to data in buffer, may be null if no data is available
 */
static void k64f_rx_update(uint8_t *buf)
{
    if (buf != NULL) {
        k64f_enet.handle.rxBdCurrent->buffer = buf;
    }

    /* Clears status. */
    k64f_enet.handle.rxBdCurrent->control &= ENET_BUFFDESCRIPTOR_RX_WRAP_MASK;

    /* Sets the receive buffer descriptor with the empty flag. */
    k64f_enet.handle.rxBdCurrent->control |= ENET_BUFFDESCRIPTOR_RX_EMPTY_MASK;

    /* Increases the buffer descriptor to the next one. */
    if (k64f_enet.handle.rxBdCurrent->control & ENET_BUFFDESCRIPTOR_RX_WRAP_MASK) {
        k64f_enet.handle.rxBdCurrent = k64f_enet.handle.rxBdBase;
        k64f_enet.handle.rxBdDirty = k64f_enet.handle.rxBdBase;
    } else {
        k64f_enet.handle.rxBdCurrent++;
        k64f_enet.handle.rxBdDirty++;
    }

    /* Actives the receive buffer descriptor. */
    ENET->RDAR = ENET_RDAR_RDAR_MASK;
}

/** \brief  Attempt to read a packet from the EMAC interface.
 *
 */
static void k64f_rx_input(void *param)
{
  int i = k64f_enet.rx_index % ENET_RX_RING_LEN;
  k64f_enet.rx_index += 1;

  /* Determine if a frame has been received */
  if ((k64f_enet.handle.rxBdCurrent->control &
      (ENET_BUFFDESCRIPTOR_RX_TRUNC_MASK | ENET_BUFFDESCRIPTOR_RX_CRC_MASK |
       ENET_BUFFDESCRIPTOR_RX_NOOCTET_MASK | ENET_BUFFDESCRIPTOR_RX_LENVLIOLATE_MASK
      )) != 0) {
#if LINK_STATS
    if ((k64f_enet.handle.rxBdCurrent->control & ENET_BUFFDESCRIPTOR_RX_LENVLIOLATE_MASK) != 0) {
      LINK_STATS_INC(link.lenerr);
    } else {
      LINK_STATS_INC(link.chkerr);
    }
#endif
    LINK_STATS_INC(link.drop);

    /* Re-use the same buffer in case of error */
    k64f_rx_update(NULL);
    return;
  }

  /* A packet is waiting, get length */
  uint32_t length = k64f_enet.handle.rxBdCurrent->length;

  /* Zero-copy */
  struct pbuf *p = k64f_enet.rx_buff[i];
  p->len = length;

  /* Attempt to queue new buffer */
  struct pbuf *rxp = pbuf_alloc(PBUF_RAW, ENET_ETH_MAX_FLEN + ENET_BUFF_ALIGNMENT, PBUF_POOL);
  if (!rxp) {
    /* Drop frame (out of memory) */
    LINK_STATS_INC(link.drop);

    /* Re-queue the same buffer */
    k64f_rx_update(NULL);

    LWIP_DEBUGF(UDP_LPC_EMAC | LWIP_DBG_TRACE,
      ("k64f_rx_input: Packet index %d dropped for OOM\n",
      k64f_enet.rx_index % ENET_RX_RING_LEN));
    return;
  }

  /* K64F note: the next line ensures that the RX buffer is properly aligned for the K64F
     RX descriptors (16 bytes alignment). However, by doing so, we're effectively changing
     a data structure which is internal to lwIP. This might not prove to be a good idea
     in the long run, but a better fix would probably involve modifying lwIP itself */
  rxp->payload = ENET_ALIGN(rxp->payload, ENET_BUFF_ALIGNMENT);
  rxp->len = ENET_ETH_MAX_FLEN;
  rxp->tot_len = ENET_ETH_MAX_FLEN;

  k64f_enet.rx_buff[i] = rxp;
  k64f_enet.rx_ptr_buff[i] = rxp->payload;
  k64f_rx_update(k64f_enet.rx_ptr_buff[i]);

  LWIP_DEBUGF(UDP_LPC_EMAC | LWIP_DBG_TRACE,
    ("k64f_rx_input: Packet received: %p, size %"PRIu32" (index=%d)\n",
    p, length, i));
  LINK_STATS_INC(link.recv);

  /* pass all packets to ethernet_input, which decides what packets it supports */
  if (k64f_enet.netif->input(p, k64f_enet.netif) != ERR_OK) {
    LWIP_DEBUGF(NETIF_DEBUG, ("k64f_enetif_input: input error\n"));
    /* Free buffer */
    pbuf_free(p);
  }
}

/** \brief  Free TX buffers that are complete
 *
 */
static void k64f_tx_reclaim(void *param)
{
  // Traverse all descriptors, looking for the ones modified by the uDMA
  while (k64f_enet.tx_consume_index != k64f_enet.tx_produce_index &&
         !(k64f_enet.handle.txBdDirty->control & ENET_BUFFDESCRIPTOR_TX_READY_MASK)) {
    pbuf_free(k64f_enet.tx_buff[k64f_enet.tx_consume_index % ENET_TX_RING_LEN]);
    if (k64f_enet.handle.txBdDirty->control & ENET_BUFFDESCRIPTOR_TX_WRAP_MASK) {
      k64f_enet.handle.txBdDirty = k64f_enet.handle.txBdBase;
    } else {
      k64f_enet.handle.txBdDirty++;
    }

    k64f_enet.tx_consume_index += 1;
    k64f_enet.tx_buff_count += 1;
  }
}

/** \brief  Output new pbuf onto the EMAC interface
 *
 *  \param[in]      buf  Pointer to pbuf to send, will free when transaction is complete
 */
static void k64f_tx_output(void *param)
{
  struct pbuf *txp = param;

  /* Check if we have enough hardware buffer descriptors. If not, we just drop the
     packet as though it was lost over the network. */
  if (k64f_enet.tx_buff_count <= 0) {
    pbuf_free(txp);
    return;
  }
  k64f_enet.tx_buff_count -= 1;

  /* Save the buffer so that it can be freed when transmit is done */
  k64f_enet.tx_buff[k64f_enet.tx_produce_index % ENET_TX_RING_LEN] = txp;
  k64f_enet.tx_produce_index += 1;

  /* Setup transfers */
  k64f_enet.handle.txBdCurrent->buffer = txp->payload;
  k64f_enet.handle.txBdCurrent->length = txp->tot_len;
  k64f_enet.handle.txBdCurrent->control |=
      (ENET_BUFFDESCRIPTOR_TX_READY_MASK | ENET_BUFFDESCRIPTOR_TX_LAST_MASK);

  /* Increase the buffer descriptor address. */
  if (k64f_enet.handle.txBdCurrent->control & ENET_BUFFDESCRIPTOR_TX_WRAP_MASK) {
    k64f_enet.handle.txBdCurrent = k64f_enet.handle.txBdBase;
  } else {
    k64f_enet.handle.txBdCurrent++;
  }

  /* Active the transmit buffer descriptor. */
  ENET->TDAR = ENET_TDAR_TDAR_MASK;
  LINK_STATS_INC(link.xmit);
}

/** \brief  Dispatch the k64f enet event queue
 */
static void k64f_dispatch(void *param)
{
  equeue_dispatch(&k64f_enet.queue, -1);
}

/** \brief  Handle hardware interrupts
 *
 *  \param[in]      base  Base pointer of enet interface
 *  \param[in]      handle  Enet interface handle
 *  \param[in]      event  Event that triggered the callback
 *  \param[in]      param  Optional parameter passed to the callback
 */
static void k64f_enet_callback(ENET_Type *base,
    enet_handle_t *handle, enet_event_t event, void *param)
{
  switch (event) {
    case kENET_RxEvent:
      equeue_call(&k64f_enet.queue, k64f_rx_input, &k64f_enet);
      break;
    case kENET_TxEvent:
      equeue_call(&k64f_enet.queue, k64f_tx_reclaim, &k64f_enet);
      break;
    default:
      break;
  }
}


/** \brief  Low level init of the EMAC
 *
 *  \param[in]      netif  Pointer to LWIP netif structure
 *  \return         ERR_OK if low level initialization was successful
 */
err_t k64f_enet_init(struct netif *netif)
{
  /* Keep netif structure around for callbacks into lwip */
  netif->state = &k64f_enet;
  k64f_enet.netif = netif;

  /* Setup descriptors */
  memset((void *)k64f_enet.tx_desc_buff, 0, sizeof(k64f_enet.tx_desc_buff));
  memset((void *)k64f_enet.rx_desc_buff, 0, sizeof(k64f_enet.rx_desc_buff));

  /* Create buffers for each receive BD */
  for (int i = 0; i < ENET_RX_RING_LEN; i++) {
    struct pbuf *rxp = pbuf_alloc(PBUF_RAW, ENET_ETH_MAX_FLEN + ENET_BUFF_ALIGNMENT, PBUF_POOL);
    if (!rxp) {
      return ERR_MEM;
    }

    /* K64F note: the next line ensures that the RX buffer is properly aligned for the K64F
       RX descriptors (16 bytes alignment). However, by doing so, we're effectively changing
       a data structure which is internal to lwIP. This might not prove to be a good idea
       in the long run, but a better fix would probably involve modifying lwIP itself */
    rxp->payload = ENET_ALIGN(rxp->payload, ENET_BUFF_ALIGNMENT);
    rxp->len = ENET_ETH_MAX_FLEN;
    rxp->tot_len = ENET_ETH_MAX_FLEN;

    k64f_enet.rx_buff[i] = rxp;
    k64f_enet.rx_ptr_buff[i] = rxp->payload;
  }

  k64f_enet.tx_consume_index = 0;
  k64f_enet.tx_produce_index = 0;
  k64f_enet.tx_buff_count = ENET_TX_RING_LEN;
  k64f_enet.rx_index = 0;

  /* Initialize phy and get hardware configuration */
  uint32_t sys_clock;
  phy_speed_t phy_speed;
  phy_duplex_t phy_duplex;

  err_t err = k64f_phy_init(&sys_clock, &phy_speed, &phy_duplex);
  if (err != ERR_OK) {
    return err;
  }

  enet_config_t config;
  ENET_GetDefaultConfig(&config);
  config.miiSpeed = (enet_mii_speed_t)phy_speed;
  config.miiDuplex = (enet_mii_duplex_t)phy_duplex;
  config.interrupt = kENET_RxFrameInterrupt | kENET_TxFrameInterrupt;
  config.rxMaxFrameLen = ENET_ETH_MAX_FLEN;
  config.macSpecialConfig = kENET_ControlFlowControlEnable;
  config.txAccelerConfig = kENET_TxAccelIsShift16Enabled;
  config.rxAccelerConfig = kENET_RxAccelisShift16Enabled | kENET_RxAccelMacCheckEnabled;

  /* prepare the buffer configuration. */
  const enet_buffer_config_t buffer_config = {
    ENET_RX_RING_LEN,
    ENET_TX_RING_LEN,
    (uint32_t)ENET_ALIGN(ENET_ETH_MAX_FLEN, ENET_BUFF_ALIGNMENT),
    0,
    k64f_enet.rx_desc_buff,
    k64f_enet.tx_desc_buff,
    (uint8_t*)k64f_enet.rx_ptr_buff,
    NULL,
  };

  /* Initialize the ENET driver */
  ENET_Init(ENET, &k64f_enet.handle, &config, &buffer_config, netif->hwaddr, sys_clock);
  ENET_SetCallback(&k64f_enet.handle, k64f_enet_callback, netif);
  ENET_ActiveRead(ENET);

  /* Setup the event queue to dispatch events in the "k64f_event_thread" */
  equeue_create_inplace(&k64f_enet.queue, sizeof(k64f_enet.event_buff), k64f_enet.event_buff);
  equeue_call_every(&k64f_enet.queue, PHY_TASK_PERIOD_MS, k64f_phy_task, netif);
  sys_thread_new("k64f_event_thread", k64f_dispatch, &k64f_enet,
      DEFAULT_THREAD_STACKSIZE, osPriorityNormal);

  return ERR_OK;
}

/** \brief  Low level output of packet to the EMAC
 *
 *  \param[in]      netif  Pointer to LWIP netif structure
 *  \param[in]      p  Packet to send out on the link
 *  \return         ERR_OK if packet has been sent on the link or ERR_MEM
 *                  if not enough memory is available to buffer the packet
 */
err_t k64f_enet_output(struct netif *netif, struct pbuf *p)
{
  struct pbuf *txp = pbuf_alloc(PBUF_RAW, p->tot_len + ENET_BUFF_ALIGNMENT, PBUF_POOL);
  if (!txp) {
    return ERR_MEM;
  }

  /* K64F note: the next line ensures that the RX buffer is properly aligned for the K64F
     RX descriptors (16 bytes alignment). However, by doing so, we're effectively changing
     a data structure which is internal to lwIP. This might not prove to be a good idea
     in the long run, but a better fix would probably involve modifying lwIP itself */
  txp->payload = ENET_ALIGN(txp->payload, ENET_BUFF_ALIGNMENT);
  txp->len = p->tot_len;
  txp->tot_len = p->tot_len;

  err_t err = pbuf_copy(txp, p);
  if (err != ERR_OK) {
    return err;
  }

  /* Defer hardware updates to the enet event queue to synchronize with the enet thread.
     After this call any loss due to insufficient buffering is considered normal packet
     loss on the network, and higher-level network protocols are responsible for
     recovery */
  equeue_call(&k64f_enet.queue, k64f_tx_output, txp);
  return ERR_OK;
}

