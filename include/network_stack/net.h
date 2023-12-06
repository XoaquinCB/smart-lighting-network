#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * A network address. Can be any value between 0-255 (inclusive).
 */
typedef uint8_t net_address;

/**
 * The type of checksum generated on a network packet.
 */
typedef enum {
    NET_CHECKSUM_NONE = 0b00,
    NET_CHECKSUM_EVEN_PARITY = 0b01,
} net_checksum_type;

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
 * @param own_address: The network address that this device should use. This should be picked carefully to avoid
 *                     multiple devices on the same network from having the same address.
 * @param checksum_type: The type of checksum to generate on all outgoing packets.
 */
void net_initialise(net_address own_address, net_checksum_type checksum_type);

/**
 * @brief Updates the network layer logic. This should be called periodically.
 */
void net_update();

/**
 * @brief Returns this device's network address, as set by the 'net_initialise()' function.
 * @returns The network address.
 */
net_address net_get_own_address();

/**
 * @brief Returns whether a given device is  known to be connected to the network.
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
 * @brief Allocates memory for a data buffer inside a network packet. Any data can be placed in the returned buffer, and
 *        it can be sent with 'net_send_data_buffer()'. If after creating a buffer, it no longer needs to be sent, it
 *        must instead be discarded with 'net_discard_data_buffer()' to free up the memory.
 * @param length: How long to make the buffer, in bytes. Buffers with zero length or longer than 121 bytes will always
 *                fail to be allocated.
 * @returns A pointer to the allocated buffer, or 'NULL' if it couldn't be allocated. If allocation fails, it should be
 *          tried again later when memory may have become available.
 */
uint8_t *net_create_data_buffer(uint8_t length);

/**
 * @brief Sends out a data buffer to the specified network address and then discards the buffer.
 * @param buffer: The buffer to send. This must be a buffer that was allocated with 'net_create_data_buffer()'; using a
 *                buffer from anywhere else will result in undefined behaviour. Once sent, the buffer is automatically
 *                discarded and must not be used again. Passing a value of 'NULL' will have no effect and nothing will
 *                be sent.
 * @param destination: The address to send the data to.
 */
void net_send_data_buffer(uint8_t *buffer, net_address destination);

/**
 * @brief Discards a data buffer without sending it, freeing up the memory for new buffers.
 * @param buffer: The buffer to discard. This must be a buffer that was allocated with 'net_create_data_buffer()'; using
 *                a buffer from anywhere else will result in undefined behaviour. Once discarded, the buffer becomes
 *                invalid and must not be used again. Passing a value of 'NULL' will have no effect.
 * @note A buffer should only be discarded if it has NOT been sent with 'net_send_data_buffer()' since
 *       'net_send_data_buffer()' already automatically discards packets.
 */
void net_discard_data_buffer(uint8_t *buffer);
