#include "routing.h"
#include "time.h"
#include "packets.h"
#include <stdbool.h>

#define NET_MAX_ADDRESS ((net_address) 15)
#define LINK_STATE_SECONDS_TO_LIVE_START (60)
#define NEIGHBOUR_LINK_SECONDS_TO_LIVE_START (60)

typedef struct {
    uint8_t sequence_number; // The sequence number of the packet that carried this link state
    uint8_t seconds_to_live; // The number of seconds left before this link state is invalid
    uint16_t connected_addresses; // Which network addresses are connected to the node - each bit corresponds to a network address.
} net_link_state_packet;

typedef struct {
    dll_address physical_address; // The physical address of the linked node
    uint8_t seconds_to_live; // The number of seconds left before this link is invalid
} net_neighbour_link;

// List of every node's link state packet. Indexed by the node's network address.
static net_link_state_packet link_state_packets[NET_MAX_ADDRESS + 1] = { 0 };

// List of neighbouring links. Indexed by the node's network address.
static net_neighbour_link neighbour_links[NET_MAX_ADDRESS + 1] = { 0 };

// List of next hops corresponding to each destination node. Indexed by the destination node's network address.
static dll_address next_hops[NET_MAX_ADDRESS + 1] = { 0 };

// Flag to signal whether the network graph has changed and the routes should be recalculated.
static bool is_graph_changed = false;

void net_initialise_routing() {
    for (net_address node = 0; node <= NET_MAX_ADDRESS; node++) {
        // Invalidate all link state packets:
        link_state_packets[node].seconds_to_live = 0;

        // Invalidate all neighbouring links:
        neighbour_links[node].seconds_to_live = 0;

        // Set all routes to unresolved:
        next_hops[node] = NET_NEXT_HOP_NOT_RESOLVED;
    }

    // Remove all connections from our link state packet:
    link_state_packets[net_get_own_address()].connected_addresses = 0;
}

static void recalculate_routes() {
    // Run Dijkstra's Algorithm to find the shortest path to all nodes in the network graph...

    typedef struct {
        uint8_t hop_count; // The number of hops between us and the destination node.
        net_address previous_node; // The previous node before the destination node.
        bool is_explored;
    } net_route;

    net_route node_routes[NET_MAX_ADDRESS + 1];
    const uint8_t hop_count_infinity = 255;

    // Initialise all the routes:
    for (net_address node = 0; node <= NET_MAX_ADDRESS; node++) {
        // Set the distance to infinity:
        node_routes[node].hop_count = hop_count_infinity;

        // Set the previous node to itself to signify that it hasn't been explored yet:
        // node_routes[node].previous_node = node;
        node_routes[node].is_explored = false;
    }
    node_routes[net_get_own_address()].hop_count = 0;

    // Start with our own node:
    net_address current_node = net_get_own_address();
    uint8_t current_hop_count = 0;

    do {
        // For all the current node's connected nodes, update their shortest path:
        node_routes[current_node].is_explored = true;
        for (net_address connected_node = 0; connected_node <= NET_MAX_ADDRESS; connected_node++) {
            // Check that that node is connected:
            if (net_are_nodes_linked(current_node, connected_node)) {
                // If this path is shorter than the node's current shortest path:
                if (current_hop_count + 1 < node_routes[connected_node].hop_count) {
                    // Update the shortest path:
                    node_routes[connected_node].hop_count = current_hop_count + 1;
                    node_routes[connected_node].previous_node = current_node;
                }
            }
        }

        // Find the unexplored node with the smallest hop count to explore in the next iteration:
        current_hop_count = hop_count_infinity;
        for (net_address node = 0; node <= NET_MAX_ADDRESS; node++) {
            // Check that the node hasn't already been explored:
            // if (node_routes[node].previous_node == node) {
            if (node_routes[node].is_explored == false) {
                // Check if the hop count of this node is shorter:
                if (node_routes[node].hop_count < current_hop_count) {
                    // Keep track of the current shortest distance and the corresponding node:
                    current_hop_count = node_routes[node].hop_count;
                    current_node = node;
                }
            }
        }
    }
    // Stop when all nodes have been explored:
    while (current_hop_count != hop_count_infinity);

    // Get the next hop for every destination address:
    net_address own_address = net_get_own_address();
    for (net_address destination = 0; destination < NET_MAX_ADDRESS; destination++) {
        // Check whether the destination node was explored and is not our own address:
        if (destination != own_address && node_routes[destination].is_explored == true) {
            net_address source = own_address;
            net_address current_node = destination;

            // Back-track from the destination node until we're one hop away from the source node:
            while (node_routes[current_node].previous_node != source) {
                current_node = node_routes[current_node].previous_node;
            }

            // Store the next hop:
            dll_address next_hop = neighbour_links[current_node].physical_address;
            next_hops[destination] = next_hop;
        } else {
            // The desination node was not explored so a route to it could not be resolved:
            next_hops[destination] = NET_NEXT_HOP_NOT_RESOLVED;
        }
    }
}

void net_update_routing() {
    static time last_time = TIME_ZERO;
    uint8_t seconds_elapsed = time_delta_seconds(last_time, time_now());
    last_time = time_add_seconds(last_time, seconds_elapsed);

    // Check timeouts:
    for (net_address address = 0; address <= NET_MAX_ADDRESS; address++) {
        // Check if the address' link state packet has timed out:
        if (link_state_packets[address].seconds_to_live > seconds_elapsed) {
            // Decrement seconds to live:
            link_state_packets[address].seconds_to_live -= seconds_elapsed;
        } else if (link_state_packets[address].seconds_to_live > 0) {
            // Link state packet has timed out:
            link_state_packets[address].seconds_to_live = 0;

            // Mark the network graph as changed:
            is_graph_changed = true;
        }

        // Check if the link to the address has timed out:
        if (neighbour_links[address].seconds_to_live > seconds_elapsed) {
            // Decrement seconds to live:
            neighbour_links[address].seconds_to_live -= seconds_elapsed;
        } else if (neighbour_links[address].seconds_to_live > 0) {
            // The link has timed out:
            neighbour_links[address].seconds_to_live = 0;

            // Remove the link from the connected addresses:
            link_state_packets[net_get_own_address()].connected_addresses &= ~((uint16_t) 1 << address);

            // Mark the network graph as changed:
            is_graph_changed = true;
        }
    }

    // Every 10 seconds:
    static uint8_t seconds_counter = 0;
    seconds_counter += seconds_elapsed;
    if (seconds_counter >= 10) {
        seconds_counter -= 10;

        // If the network graph has changed, update the routes:
        if (is_graph_changed == true) {
            // is_graph_changed = false;
            recalculate_routes();
        }

        // Send out a ping request packet and a link state packet to all neighbouring nodes:
        net_send_ping_request_packet(DLL_BROADCAST_ADDRESS);
        net_send_link_state_packet();
    }
}

bool net_are_nodes_linked(net_address node_1, net_address node_2) {
    // Make sure the addresses are within limits:
    if (node_1 > NET_MAX_ADDRESS || node_2 > NET_MAX_ADDRESS) {
        return false;
    }

    // Check if the link state has timed out (our own link state packet never times out):
    if (link_state_packets[node_1].seconds_to_live == 0 && node_1 != net_get_own_address()) {
        return false;
    }

    // Read the relevant bit to check if the nodes are linked:
    return link_state_packets[node_1].connected_addresses & (1 << node_2);
}

bool net_is_device_online(net_address address) {
    // Make sure the address is within limits:
    if (address > NET_MAX_ADDRESS) {
        return false;
    }

    // Our own address is always online:
    if (address == net_get_own_address()) {
        return true;
    }

    // If the link state packet for the given node hasn't timed out, assume the node is online:
    bool is_online = link_state_packets[address].seconds_to_live > 0;
    return is_online;
}

bool net_notify_link_state_packet(net_address source, uint8_t sequence_number, const net_address *node_list, uint8_t node_count) {
    // Make sure the address is within limits and isn't our own address:
    if (source > NET_MAX_ADDRESS || source == net_get_own_address()) {
        return false;
    }

    // Check that the sequence number is valid:
    if (link_state_packets[source].seconds_to_live != 0) {
        uint8_t previous_sequence_number = link_state_packets[source].sequence_number;
        uint8_t sequence_number_difference = sequence_number - previous_sequence_number;
        if (sequence_number_difference == 0 || sequence_number_difference > 128) {
            return false;
        }
    }

    // Reset the seconds to live and update the sequence number:
    link_state_packets[source].seconds_to_live = LINK_STATE_SECONDS_TO_LIVE_START;
    link_state_packets[source].sequence_number = sequence_number;

    // Set the bits for all the connected addresses:
    uint16_t connected_addresses = 0;
    for (uint8_t node_index = 0; node_index < node_count; node_index++) {
        net_address address = node_list[node_index];
        if (address <= NET_MAX_ADDRESS) {
            connected_addresses |= ((uint16_t) 1 << address);
        }
    }

    // Check if the connected addresses have changed:
    if (connected_addresses != link_state_packets[source].connected_addresses) {
        // Store the connected addresses:
        link_state_packets[source].connected_addresses = connected_addresses;

        // Mark the network graph as changed:
        is_graph_changed = true;
    }

    // The packet was valid:
    return true;
}

dll_address net_get_next_hop(net_address destination) {
    if (destination > NET_MAX_ADDRESS) {
        return NET_NEXT_HOP_NOT_RESOLVED;
    }
    return next_hops[destination];
}

void net_notify_ping_response(dll_address physical_address, net_address logical_address) {
    // Make sure the address is within limits:
    if (logical_address > NET_MAX_ADDRESS) {
        return;
    }

    // Reset the time to live and update the physical address:
    neighbour_links[logical_address].seconds_to_live = NEIGHBOUR_LINK_SECONDS_TO_LIVE_START;
    neighbour_links[logical_address].physical_address = physical_address;

    // Add the link to the connected addresses:
    net_address own_address = net_get_own_address();
    uint16_t connected_addresses = link_state_packets[own_address].connected_addresses | ((uint16_t) 1 << logical_address);

    // Check if the connected addresses are different to before:
    if (connected_addresses != link_state_packets[own_address].connected_addresses) {
        // Store the connected addresses:
        link_state_packets[own_address].connected_addresses = connected_addresses;

        // Mark the network graph as changed:
        is_graph_changed = true;
    }
}

bool net_is_node_neighbour(dll_address physical_address) {
    // Search through the neighbouring links:
    for (net_address logical_address = 0; logical_address < NET_MAX_ADDRESS; logical_address++) {
        // If the physical address matches the requested one, and the link hasn't timed out, return 'true':
        if (neighbour_links[logical_address].physical_address == physical_address
            && neighbour_links[logical_address].seconds_to_live > 0) {
            return true;
        }
    }
    // Link wasn't found:
    return false;
}
