#pragma once

#include "network_stack/net.h"
#include "network_stack/dll.h"

/**
 * Value returned by 'net_get_next_hop()' if the next hop can't be resolved.
 */
#define NET_NEXT_HOP_NOT_RESOLVED (DLL_BROADCAST_ADDRESS)

/**
 * @brief Returns the physical address of the next node to send a packet to, given a destination logical address.
 * @param destination: The intended final destination of a packet.
 * @returns The physical address of the next node to send the packet to. If the next hop can't be resolved
 *          'NET_NEXT_HOP_NOT_RESOLVED' is returned.
 */
dll_address net_get_next_hop(net_address destination);

// TODO: Comment this function
void net_initialise(routing);

/**
 * @brief Updates the routing, sending out ping requests and link state packets, and keeping track of links and routes.
 */
void net_update_routing();

/**
 * @brief Notifies the router that a ping response was received from a node.
 * @param physical_address: The physical address of the node that send the response.
 * @param logical_address: The logical address of the node that sent the response.
 */
void net_notify_ping_response(dll_address physical_address, net_address logical_address);

/**
 * @brief Notifies the router that a link state packet was received.
 * @param source: The node that sent out the link state packet.
 * @param sequence_number: The packet's sequence number.
 * @param node_list: A pointer to the first node in the packet's node list.
 * @param node_count: The number of nodes in the list.
 * @returns 'true' if the packet is valid (it hasn't been received already) and should be flooded; 'false' otherwise.
 */
bool net_notify_link_state_packet(net_address source, uint8_t sequence_number, const net_address *node_list, uint8_t node_count);

/**
 * @brief Returns whether two nodes are directly linked.
 * @param node_1: The link's starting node.
 * @param node_2: The link's ending node.
 * @returns 'true' if the two nodes are directly linked or 'false' otherwise.
 */
bool net_are_nodes_linked(net_address node_1, net_address node_2);

/**
 * @brief Returns whether a given device is  known to be connected to the network.
 * @param address: The device's network address.
 * @returns 'true' if the device is online or 'false' otherwise.
 */
bool net_is_device_online(net_address address);

/**
 * @brief Returns whether a node is a neighbour to this node.
 * @param node: The physical address of the node.
 * @returns 'true' if the node is a neighbour; 'false' otherwise.
 */
bool net_is_node_neighbour(dll_address node);
