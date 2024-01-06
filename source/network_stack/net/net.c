#include "network_stack/net.h"
#include "network_stack/dll.h"
#include "packets.h"
#include "routing.h"

// Note: Not all the functions declared in net.h are implemented in this file. The other functions are implemented in
//       separate source files.

// This device's network address.
#define NET_OWN_ADDRESS ((net_address) 0x00)

void net_initialise() {
    dll_set_callback(net_handle_received_packet);
    net_initialise_routing();
}

void net_update() {
    net_update_routing();
}

net_address net_get_own_address() {
    return NET_OWN_ADDRESS;
}
