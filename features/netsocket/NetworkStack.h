// Stub for backwards compatibility

#ifndef NSAPI_NETWORKSTACK_H
#define NSAPI_NETWORKSTACK_H

#include "netsocket/NetworkInterface.h"
#warning "NetworkStack has been deprecated, please update NetworkStack implementations to implement NetworkInterface directly [since mbed-os-5.2.1]"

class NetworkStack {};

#endif
