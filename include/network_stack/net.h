#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * Represents a network layer address.
 */
typedef uint8_t net_address_t;

/**
 * Represents the checksum mode used on a network packet.
 */
typedef enum {
    NET_CHECKSUM_NONE = 0b00,
    NET_CHECKSUM_EVEN_PARITY = 0b01,
} net_checksum_mode_t;

/**
 * @brief Callback function pointer for receiving a network packet.
 * @param source: The source address that the packet came from.
 * @param data: A pointer to the data received. Note that after this function returns this pointer is no longer valid,
 *              so if the data is needed after the function returns it must be copied to a separate buffer.
 * @param data_length: The number of bytes of data. This will not be larger than 121.
 */
typedef void (*net_receive_callback_t)(net_address_t source, const uint8_t *data, uint8_t data_length);

/**
 * @brief Initialises the network layer.
 * @param own_address: The network address that this device should use.
 */
void net_initialise(net_address_t own_address);

/**
 * @brief Returns whether a node is connected to the network.
 * @param node: The node's address.
 */
bool net_is_node_online(net_address_t address);

/**
 * @brief Sets the user function to be called when a packet is received.
 * @param callback: The function to call. Set to 'NULL' to use no callback.
 */
void net_receive_packet_callback(net_receive_callback_t callback);

/**
 * @brief Attempts to send a packet to the specified network address.
 * @param destination: The address to send the packet to.
 * @param checksum_mode: The type of checksum to generate on the packet.
 * @param data: A pointer to the data to send.
 * @param data_length: The number of bytes of data. Must not be larger than 121.
 * @return 'true' if the packet was send; 'false' if there was some error. Note that this doesn't confirm that the
 *         destination received the packet.
 */
bool net_send_packet(net_address_t destination, net_checksum_mode_t checksum_mode, const uint8_t *data, uint8_t data_length);
