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
#include "fsl_port.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



/* State of the enet PHY */
struct k64f_phy_state {
    int connected;
    phy_speed_t speed;
    phy_duplex_t duplex;
};

static struct k64f_phy_state 
    k64f_phy_state_crt  = {-1, (phy_speed_t)-1, (phy_duplex_t)-1},
    k64f_phy_state_prev = {-1, (phy_speed_t)-1, (phy_duplex_t)-1};


// Predeclaration of hardware initialization function
static void k64f_init_eth_hardware(void);


/** \brief  Low level init of the PHY
 *
 *  \param[in]      netif  Pointer to LWIP netif structure
 *  \return         ERR_OK if low level initialization was successful
 */
err_t k64f_phy_init(uint32_t *sys_clock, phy_speed_t *phy_speed, phy_duplex_t *phy_duplex)
{
  LWIP_ASSERT("sys_clock/phy_speed/phy_duplex != NULL",
      (sys_clock != NULL && phy_speed != NULL && phy_duplex != NULL));

  extern void k64f_init_eth_hardware(void);
  k64f_init_eth_hardware();

  *sys_clock = CLOCK_GetFreq(kCLOCK_CoreSysClk);

  PHY_Init(ENET, PHY_ADDR, *sys_clock);

  bool link;
  PHY_GetLinkStatus(ENET, PHY_ADDR, &link);
  if (link) {
    PHY_GetLinkSpeedDuplex(ENET, PHY_ADDR, phy_speed, phy_duplex);
  }

  return ERR_OK;
}

/** \brief  Get the current status of the PHY
 *
 *  \return         true if the link is up
 */
bool k64f_phy_link_status() {
    bool connection_status;
    PHY_GetLinkStatus(ENET, PHY_ADDR, &connection_status);
    return (int)connection_status;
}

/** \brief  Periodic PHY task
 *
 *  Must be called periodically by the enet driver at PHY_TASK_PERIOD_MS milliseconds
 *
 *  \param[in]      param  Optional argument to pass to the netif_set_link_up and
 *                  netif_set_link_down functions called on phy updates
 */
void k64f_phy_task(void *param) {
  // Get current status
  bool connection_status;
  PHY_GetLinkStatus(ENET, PHY_ADDR, &connection_status);
  k64f_phy_state_crt.connected = connection_status ? 1 : 0;
  // Get the actual PHY link speed
  PHY_GetLinkSpeedDuplex(ENET, PHY_ADDR, &k64f_phy_state_crt.speed, &k64f_phy_state_crt.duplex);

  // Compare with previous state
  if (k64f_phy_state_crt.connected != k64f_phy_state_prev.connected) {
    if (k64f_phy_state_crt.connected) {
      tcpip_callback_with_block((tcpip_callback_fn)netif_set_link_up, param, 1);
    } else {
      tcpip_callback_with_block((tcpip_callback_fn)netif_set_link_down, param, 1);
    }
  }

  if (k64f_phy_state_crt.speed != k64f_phy_state_prev.speed) {
    uint32_t rcr = ENET->RCR;
    rcr &= ~ENET_RCR_RMII_10T_MASK;
    rcr |= ENET_RCR_RMII_10T(!k64f_phy_state_crt.speed);
    ENET->RCR = rcr;
  }

  k64f_phy_state_prev = k64f_phy_state_crt;
}


/** \brief  Initialize eth hardware on the K64F
 */
void k64f_init_eth_hardware(void)
{
    port_pin_config_t configENET = {0};

#ifndef FEATURE_UVISOR
    /* Disable MPU only when uVisor is not around. */
    MPU->CESR &= ~MPU_CESR_VLD_MASK;
#endif/*FEATURE_UVISOR*/

    CLOCK_EnableClock(kCLOCK_PortC);
    CLOCK_EnableClock(kCLOCK_PortB);
    /* Affects PORTC_PCR16 register */
    PORT_SetPinMux(PORTC, 16u, kPORT_MuxAlt4);
    /* Affects PORTC_PCR17 register */
    PORT_SetPinMux(PORTC, 17u, kPORT_MuxAlt4);
    /* Affects PORTC_PCR18 register */
    PORT_SetPinMux(PORTC, 18u, kPORT_MuxAlt4);
    /* Affects PORTC_PCR19 register */
    PORT_SetPinMux(PORTC, 19u, kPORT_MuxAlt4);
    /* Affects PORTB_PCR1 register */
    PORT_SetPinMux(PORTB, 1u, kPORT_MuxAlt4);

    configENET.openDrainEnable = kPORT_OpenDrainEnable;
    configENET.mux = kPORT_MuxAlt4;
    configENET.pullSelect = kPORT_PullUp;
    /* Ungate the port clock */
    CLOCK_EnableClock(kCLOCK_PortA);
    /* Affects PORTB_PCR0 register */
    PORT_SetPinConfig(PORTB, 0u, &configENET);

    /* Affects PORTA_PCR13 register */
    PORT_SetPinMux(PORTA, 13u, kPORT_MuxAlt4);
    /* Affects PORTA_PCR12 register */
    PORT_SetPinMux(PORTA, 12u, kPORT_MuxAlt4);
    /* Affects PORTA_PCR14 register */
    PORT_SetPinMux(PORTA, 14u, kPORT_MuxAlt4);
    /* Affects PORTA_PCR5 register */
    PORT_SetPinMux(PORTA, 5u, kPORT_MuxAlt4);
    /* Affects PORTA_PCR16 register */
    PORT_SetPinMux(PORTA, 16u, kPORT_MuxAlt4);
    /* Affects PORTA_PCR17 register */
    PORT_SetPinMux(PORTA, 17u, kPORT_MuxAlt4);
    /* Affects PORTA_PCR15 register */
    PORT_SetPinMux(PORTA, 15u, kPORT_MuxAlt4);
    /* Affects PORTA_PCR28 register */
    PORT_SetPinMux(PORTA, 28u, kPORT_MuxAlt4);

    /* Select the Ethernet timestamp clock source */
    CLOCK_SetEnetTime0Clock(0x2);
}
