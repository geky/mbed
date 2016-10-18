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

#include "EthernetInterface.h"
#include "lwip_stack.h"


/* Interface implementation */
EthernetInterface::EthernetInterface()
    : _dhcp(true), _ip_address(), _netmask(), _gateway()
{
}

int EthernetInterface::set_network(const char *ip_address, const char *netmask, const char *gateway)
{
    _dhcp = false;
    strncpy(_ip_address, ip_address ? ip_address : "", sizeof(_ip_address));
    strncpy(_netmask, netmask ? netmask : "", sizeof(_netmask));
    strncpy(_gateway, gateway ? gateway : "", sizeof(_gateway));
    return 0;
}

int EthernetInterface::set_dhcp(bool dhcp)
{
    _dhcp = dhcp;
    return 0;
}

int EthernetInterface::connect()
{
    return mbed_lwip_bringup(_dhcp,
            _ip_address[0] ? _ip_address : 0,
            _netmask[0] ? _netmask : 0,
            _gateway[0] ? _gateway : 0);
}

int EthernetInterface::disconnect()
{
    return mbed_lwip_bringdown();
}

const char *EthernetInterface::get_mac_address()
{
    return mbed_lwip_get_mac_address();
}

const char *EthernetInterface::get_ip_address()
{
    if (mbed_lwip_get_ip_address(_ip_address, sizeof _ip_address)) {
        return _ip_address;
    }

    return 0;
}

const char *EthernetInterface::get_netmask()
{
    if (mbed_lwip_get_netmask(_netmask, sizeof _netmask)) {
        return _netmask;
    }

    return 0;
}

const char *EthernetInterface::get_gateway()
{
    if (mbed_lwip_get_gateway(_gateway, sizeof _gateway)) {
        return _gateway;
    }

    return 0;
}

int EthernetInterface::gethostbyname(const char *host, SocketAddress *address)
{
    nsapi_addr_t addr = {NSAPI_IPv4, 0};
    int err = mbed_lwip_gethostbyname(host, &addr);
    address->set_addr(addr);
    return err;
}

int EthernetInterface::socket_open(nsapi_socket_t *handle, nsapi_protocol_t proto)
{
    return mbed_lwip_socket_open(handle, proto);
}

int EthernetInterface::socket_close(nsapi_socket_t handle)
{
    return mbed_lwip_socket_close(handle);
}

int EthernetInterface::socket_bind(nsapi_socket_t handle, const SocketAddress &address)
{
    return mbed_lwip_socket_bind(handle, address.get_addr(), address.get_port());
}

int EthernetInterface::socket_listen(nsapi_socket_t handle, int backlog)
{
    return mbed_lwip_socket_listen(handle, backlog);
}

int EthernetInterface::socket_connect(nsapi_socket_t handle, const SocketAddress &address)
{
    return mbed_lwip_socket_connect(handle, address.get_addr(), address.get_port());
}

int EthernetInterface::socket_accept(nsapi_socket_t server, nsapi_socket_t *handle, SocketAddress *address)
{
    nsapi_addr_t addr = {NSAPI_IPv4, 0};
    uint16_t port = 0;

    int err = mbed_lwip_socket_accept(server, handle, &addr, &port);

    if (address) {
        address->set_addr(addr);
        address->set_port(port);
    }

    return err;
}

int EthernetInterface::socket_send(nsapi_socket_t handle, const void *data, unsigned size)
{
    return mbed_lwip_socket_send(handle, data, size);
}

int EthernetInterface::socket_recv(nsapi_socket_t handle, void *data, unsigned size)
{
    return mbed_lwip_socket_recv(handle, data, size);
}

int EthernetInterface::socket_sendto(nsapi_socket_t handle, const SocketAddress &address, const void *data, unsigned size)
{
    return mbed_lwip_socket_sendto(handle, address.get_addr(), address.get_port(), data, size);
}

int EthernetInterface::socket_recvfrom(nsapi_socket_t handle, SocketAddress *address, void *data, unsigned size)
{
    nsapi_addr_t addr = {NSAPI_IPv4, 0};
    uint16_t port = 0;

    int err = mbed_lwip_socket_recvfrom(handle, &addr, &port, data, size);

    if (address) {
        address->set_addr(addr);
        address->set_port(port);
    }

    return err;
}

int EthernetInterface::setsockopt(nsapi_socket_t handle, int level, int optname, const void *optval, unsigned optlen)
{
    return mbed_lwip_setsockopt(handle, level, optname, optval, optlen);
}

void EthernetInterface::socket_attach(nsapi_socket_t handle, void (*callback)(void *), void *data)
{
    return mbed_lwip_socket_attach(handle, callback, data);
}
