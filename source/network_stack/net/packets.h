#pragma once

#include "network_stack/dll.h"
#include "network_stack/net.h"
#include <stdint.h>

/**
 * @brief Returns a buffer which is used for writing data packets. The size of the buffer can be retrived with
 *        'net_get_data_buffer_size()'.
 * @returns A pointer to the first byte in the buffer.
 */
uint8_t *net_get_data_buffer();

/**
 * @brief Returns the size of the data buffer.
 * @returns The size in bytes.
 */
uint8_t net_get_data_buffer_size();

/**
 * @brief Sends a data packet, with whatever data is in the data buffer, to the given destination.
 * @param destination: The logical address to send the packet to.
 * @param data_length: The number of bytes to send from the data buffer.
 */
void net_send_data_packet(net_address destination, uint8_t data_length);

/**
 * @brief Sends out a new link state packet to the entire network, with information about this node's links.
 */
void net_send_link_state_packet();

/**
 * @brief Sends a ping request packet to a neighbouring node.
 * @param node: The node to send the ping request to.
 */
void net_send_ping_request_packet(dll_address node);

/**
 * @brief Sends a ping response packet to a neighbouring node.
 * @param node: The node to send the ping request to.
 */
void net_send_ping_response_packet(dll_address node);

/**
 * @brief Handles a received network packet.
 * @param previous_hop: The node which the packet was directly received from.
 * @param packet: The packet's bytes.
 * @param packet_length: The number of bytes in the packet.
 */
void net_handle_received_packet(dll_address previous_hop, uint8_t *packet, uint8_t packet_length);

/**
 * @brief Sets the user function to be called when a network data packet is received.
 * @param callback: The function to be called. Set to 'NULL' to not use a callback (default).
 */
void net_set_receive_callback(net_receive_callback callback);
