#include "k64f_eth.h"
#include "k64f_enet.h"

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

#include "mbed_interface.h"


#if LWIP_IPV4
/**
 * This function is the ipv4 ethernet packet send function. It calls
 * etharp_output after checking link status.
 *
 * \param[in] netif the lwip network interface structure for this enetif
 * \param[in] q Pointer to pbuf to send
 * \param[in] ipaddr IP address
 * \return ERR_OK or error code
 */
static err_t k64f_etharp_output_ipv4(struct netif *netif, struct pbuf *q, const ip4_addr_t *ipaddr)
{
  /* Only send packet is link is up */
  if (netif->flags & NETIF_FLAG_LINK_UP) {
    return etharp_output(netif, q, ipaddr);
  }

  return ERR_CONN;
}
#endif

#if LWIP_IPV6
/**
 * This function is the ipv6 ethernet packet send function. It calls
 * ethip6_output after checking link status.
 *
 * \param[in] netif the lwip network interface structure for this enetif
 * \param[in] q Pointer to pbug to send
 * \param[in] ipaddr IP address
 * \return ERR_OK or error code
 */
static err_t k64f_etharp_output_ipv6(struct netif *netif, struct pbuf *q, const ip6_addr_t *ipaddr)
{
  /* Only send packet is link is up */
  if (netif->flags & NETIF_FLAG_LINK_UP) {
    return ethip6_output(netif, q, ipaddr);
  }

  return ERR_CONN;
}
#endif

#if LWIP_IGMP
/**
 * IPv4 address filtering setup.
 *
 * \param[in] netif the lwip network interface structure for this enetif
 * \param[in] group IPv4 group to modify
 * \param[in] action
 * \return ERR_OK or error code
 */
static err_t igmp_mac_filter(struct netif *netif, const ip4_addr_t *group, u8_t action)
{
    switch (action) {
        case IGMP_ADD_MAC_FILTER:
        {
            uint32_t group23 = ntohl(group->addr) & 0x007FFFFF;
            uint8_t addr[6];
            addr[0] = LL_IP4_MULTICAST_ADDR_0;
            addr[1] = LL_IP4_MULTICAST_ADDR_1;
            addr[2] = LL_IP4_MULTICAST_ADDR_2;
            addr[3] = group23 >> 16;
            addr[4] = group23 >> 8;
            addr[5] = group23;
            ENET_AddMulticastGroup(ENET, addr);
            return ERR_OK;
        }
        case IGMP_DEL_MAC_FILTER:
            /* As we don't reference count, silently ignore delete requests */
            return ERR_OK;
        default:
            return ERR_ARG;
    }
}
#endif

#if LWIP_IPV6_MLD
/**
 * IPv6 address filtering setup.
 *
 * \param[in] netif the lwip network interface structure for this enetif
 * \param[in] group IPv6 group to modify
 * \param[in] action
 * \return ERR_OK or error code
 */
static err_t mld_mac_filter(struct netif *netif, const ip6_addr_t *group, u8_t action)
{
    switch (action) {
        case MLD6_ADD_MAC_FILTER:
        {
            uint32_t group32 = ntohl(group->addr[3]);
            uint8_t addr[6];
            addr[0] = LL_IP6_MULTICAST_ADDR_0;
            addr[1] = LL_IP6_MULTICAST_ADDR_1;
            addr[2] = group32 >> 24;
            addr[3] = group32 >> 16;
            addr[4] = group32 >> 8;
            addr[5] = group32;
            ENET_AddMulticastGroup(ENET, addr);
            return ERR_OK;
        }
        case MLD6_DEL_MAC_FILTER:
            /* As we don't reference count, silently ignore delete requests */
            return ERR_OK;
        default:
            return ERR_ARG;
    }
}
#endif


/**
 * Should be called at the beginning of the program to set up the
 * network interface.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param[in] netif the lwip network interface structure for this netif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t eth_arch_enetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));

  /* set MAC hardware address */
#if (MBED_MAC_ADDRESS_SUM != MBED_MAC_ADDR_INTERFACE)
  netif->hwaddr[0] = MBED_MAC_ADDR_0;
  netif->hwaddr[1] = MBED_MAC_ADDR_1;
  netif->hwaddr[2] = MBED_MAC_ADDR_2;
  netif->hwaddr[3] = MBED_MAC_ADDR_3;
  netif->hwaddr[4] = MBED_MAC_ADDR_4;
  netif->hwaddr[5] = MBED_MAC_ADDR_5;
#else
  mbed_mac_address((char *)netif->hwaddr);
#endif

  /* Ethernet address length */
  netif->hwaddr_len = ETH_HWADDR_LEN;

  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  // TODOETH: check if the flags are correct below
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET;

  /* Initialize the hardware */
  err_t err = k64f_enet_init(netif);
  if (err != ERR_OK) {
    return err;
  }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwipk64f";
#endif

  netif->name[0] = 'e';
  netif->name[1] = 'n';

#if LWIP_IPV4
  /* Set high-level eth output function */
  netif->output = k64f_etharp_output_ipv4;
#if LWIP_IGMP
  /* Set igmp filter function */
  netif->igmp_mac_filter = igmp_mac_filter;
  netif->flags |= NETIF_FLAG_IGMP;
#endif
#endif
#if LWIP_IPV6
  /* Set high-level eth output function */
  netif->output_ip6 = k64f_etharp_output_ipv6;
#if LWIP_IPV6_MLD
  /* Set multicast listener function */
  netif->mld_mac_filter = mld_mac_filter;
  netif->flags |= NETIF_FLAG_MLD6;
#else
  /* Would need to enable all multicasts here - no API in fsl_enet to do that */
  #error "IPv6 multicasts won't be received if LWIP_IPV6_MLD is disabled, breaking the system"
#endif
#endif

  /* Set low-level link output function */
  netif->linkoutput = k64f_enet_output;

  /* Allow the PHY task to detect the initial link state and set up the proper flags */
  osDelay(10);

  return ERR_OK;
}

/**
 * Called to enable interrupts
 */
void eth_arch_enable_interrupts(void) {
  // NVIC_SetPriority(ENET_Receive_IRQn, 6U);
  // NVIC_SetPriority(ENET_Transmit_IRQn, 6U);
}

/**
 * Called to disable interrupts
 */
void eth_arch_disable_interrupts(void) {

}

