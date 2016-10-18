/* Socket
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

#include "netsocket/NetworkInterface.h"
#include "netsocket/SocketAddress.h"
#include "nsapi_dns.h"
#include <string.h>


// Default network-interface state
const char *NetworkInterface::get_mac_address()
{
    return 0;
}

const char *NetworkInterface::get_ip_address()
{
    return 0;
}

const char *NetworkInterface::get_netmask()
{
    return 0;
}

const char *NetworkInterface::get_gateway()
{
    return 0;
}

int NetworkInterface::set_network(const char *ip_address, const char *netmask, const char *gateway)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

int NetworkInterface::set_dhcp(bool dhcp)
{
    if (!dhcp) {
        return NSAPI_ERROR_UNSUPPORTED;
    } else {
        return 0;
    }
}

// DNS operations go through the underlying stack by default
int NetworkInterface::gethostbyname(const char *name, SocketAddress *address)
{
    // check for simple ip addresses
    if (address->set_ip_address(name)) {
        return 0;
    }

    return nsapi_dns_query(this, name, address);
}

int NetworkInterface::gethostbyname(const char *name, SocketAddress *address, nsapi_version_t version)
{
    // check for simple ip addresses
    if (address->set_ip_address(name)) {
        if (address->get_ip_version() != version) {
            return NSAPI_ERROR_DNS_FAILURE;
        }

        return 0;
    }

    return nsapi_dns_query(this, name, address, version);
}

int NetworkInterface::add_dns_server(const SocketAddress &address)
{
    return nsapi_dns_add_server(address);
}

// Other operations default to UNSUPPORTED
int NetworkInterface::setstackopt(int level, int optname, const void *optval, unsigned optlen)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

int NetworkInterface::getstackopt(int level, int optname, void *optval, unsigned *optlen)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

int NetworkInterface::socket_open(nsapi_socket_t *handle, nsapi_protocol_t proto)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

int NetworkInterface::socket_close(nsapi_socket_t handle)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

int NetworkInterface::socket_bind(nsapi_socket_t handle, const SocketAddress &address)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

int NetworkInterface::socket_listen(nsapi_socket_t handle, int backlog)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

int NetworkInterface::socket_connect(nsapi_socket_t handle, const SocketAddress &address)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

int NetworkInterface::socket_accept(nsapi_socket_t server, nsapi_socket_t *handle, SocketAddress *address)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

int NetworkInterface::socket_send(nsapi_socket_t handle, const void *data, unsigned size)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

int NetworkInterface::socket_recv(nsapi_socket_t handle, void *data, unsigned size)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

int NetworkInterface::socket_sendto(nsapi_socket_t handle, const SocketAddress &address, const void *data, unsigned size)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

int NetworkInterface::socket_recvfrom(nsapi_socket_t handle, SocketAddress *address, void *buffer, unsigned size)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

void NetworkInterface::socket_attach(nsapi_socket_t handle, void (*callback)(void *), void *data)
{
}

int NetworkInterface::setsockopt(nsapi_socket_t handle, int level, int optname, const void *optval, unsigned optlen)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

int NetworkInterface::getsockopt(nsapi_socket_t handle, int level, int optname, void *optval, unsigned *optlen)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

