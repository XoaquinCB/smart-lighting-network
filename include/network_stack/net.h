#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * A network address. Can be any integer between 0 and NET_MAX_ADDRESS (inclusive).
 */
typedef uint8_t net_address;

/**
 * The maximum allowed network address.
 */
#define NET_MAX_ADDRESS ((net_address) 15)

/**
 * @brief A callback function pointer for handling a received network data packet.
 * @param source: The source address that the packet came from.
 * @param payload: A pointer to the first byte in the packet's payload. Note that this pointer is only valid until the
 *                 function returns, so if the data is needed for longer it must be copied to a separate buffer. This
 *                 pointer will be 'NULL' if and only if 'length' is zero.
 * @param length: The number of bytes in the payload.
 */
typedef void (*net_receive_callback)(net_address source, uint8_t *payload, uint8_t length);

/**
 * @brief Initialises the network layer. Must be called once at the start of the program before calling any other
 *        'net_()' functions.
 */
void net_initialise();

/**
 * @brief Updates the network layer logic. This should be called periodically.
 */
void net_update();

/**
 * @brief Returns this device's statically assigned network address.
 * @returns The network address.
 */
net_address net_get_own_address();

/**
 * @brief Returns whether a given device is known to be connected to the network.
 * @param address: The device's network address.
 * @returns 'true' if the device is online or 'false' otherwise.
 */
bool net_is_device_online(net_address address);

/**
 * @brief Sets the user function to be called when a network data packet is received.
 * @param callback: The function to be called. Set to 'NULL' to not use a callback (default).
 */
void net_set_receive_callback(net_receive_callback callback);

/**
 * @brief Returns a buffer which is used for writing data packets. The size of the buffer can be retrieved with
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
