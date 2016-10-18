
/** \addtogroup netsocket */
/** @{*/
/* nsapi_dns.h
 * Original work Copyright (c) 2013 Henry Leinen (henry[dot]leinen [at] online [dot] de)
 * Modified work Copyright (c) 2015 ARM Limited
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
#ifndef NSAPI_DNS_H
#define NSAPI_DNS_H

#include "nsapi_types.h"
#include "netsocket/NetworkInterface.h"


/** Query a domain name server for an IP address of a given hostname
 *
 *  @param stack    Network stack as target for DNS query
 *  @param host     Hostname to resolve
 *  @param addr     Destination for the host address
 *  @param version  IP version to resolve (defaults to NSAPI_IPv4)
 *  @return         0 on success, negative error code on failure
 *                  NSAPI_ERROR_DNS_FAILURE indicates the host could not be found
 */
int nsapi_dns_query(NetworkInterface *stack, const char *host,
        SocketAddress *addr, nsapi_version_t version = NSAPI_IPv4);

/** Query a domain name server for multiple IP address of a given hostname
 *
 *  @param stack      Network stack as target for DNS query
 *  @param host       Hostname to resolve
 *  @param addr       Array for the host addresses
 *  @param addr_count Number of addresses allocated in the array
 *  @param version    IP version to resolve (defaults to NSAPI_IPv4)
 *  @return           Number of addresses found on success, negative error code on failure
 *                    NSAPI_ERROR_DNS_FAILURE indicates the host could not be found
 */
int nsapi_dns_query_multiple(NetworkInterface *stack, const char *host,
        SocketAddress *addr, unsigned addr_count, nsapi_version_t version = NSAPI_IPv4);

/** Add a domain name server to list of servers to query
 *
 *  @param addr     Destination for the host address
 *  @return         0 on success, negative error code on failure
 */
int nsapi_dns_add_server(const SocketAddress &address);


#endif

/** @}*/
