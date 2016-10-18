/* LWIP implementation of NetworkInterfaceAPI
 * Copyright (c) 2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LWIP_STACK_H
#define LWIP_STACK_H

#include "nsapi.h"
#include "emac_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// Access to lwip through the nsapi
int mbed_lwip_init(emac_interface_t *emac);
int mbed_lwip_bringup(bool dhcp, const char *ip, const char *netmask, const char *gw);
int mbed_lwip_bringdown(void);

const char *mbed_lwip_get_mac_address(void);
char *mbed_lwip_get_ip_address(char *buf, int buflen);
char *mbed_lwip_get_netmask(char *buf, int buflen);
char *mbed_lwip_get_gateway(char *buf, int buflen);

int mbed_lwip_gethostbyname(const char *host, nsapi_addr_t *addr);
int mbed_lwip_socket_open(nsapi_socket_t *handle, nsapi_protocol_t proto);
int mbed_lwip_socket_close(nsapi_socket_t handle);
int mbed_lwip_socket_bind(nsapi_socket_t handle, nsapi_addr_t addr, uint16_t port);
int mbed_lwip_socket_listen(nsapi_socket_t handle, int backlog);
int mbed_lwip_socket_connect(nsapi_socket_t handle, nsapi_addr_t addr, uint16_t port);
int mbed_lwip_socket_accept(nsapi_socket_t server, nsapi_socket_t *handle, nsapi_addr_t *addr, uint16_t *port);
int mbed_lwip_socket_send(nsapi_socket_t handle, const void *data, unsigned size);
int mbed_lwip_socket_recv(nsapi_socket_t handle, void *data, unsigned size);
int mbed_lwip_socket_sendto(nsapi_socket_t handle, nsapi_addr_t addr, uint16_t port, const void *data, unsigned size);
int mbed_lwip_socket_recvfrom(nsapi_socket_t handle, nsapi_addr_t *addr, uint16_t *port, void *data, unsigned size);
int mbed_lwip_setsockopt(nsapi_socket_t handle, int level, int optname, const void *optval, unsigned optlen);
void mbed_lwip_socket_attach(nsapi_socket_t handle, void (*callback)(void *), void *data);


#ifdef __cplusplus
}
#endif

#endif
