#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * The default checksum type that should be used on packets sent by this device.
 */
#define NET_TX_CHECKSUM_TYPE (NET_CHECKSUM_EVEN_PARITY)

/**
 * The type of checksum generated on a network packet.
 */
typedef enum {
    NET_CHECKSUM_NONE = 0b00,
    NET_CHECKSUM_EVEN_PARITY = 0b01,
} net_checksum_type;

/**
 * @brief Generates a checksum on some data.
 * @param checksum_type: The type of checksum to generate.
 * @param data: A pointer to the start of the data to generate a checksum on.
 * @param data_length: The number of bytes of data.
 * @returns The 16-bit checksum generated.
 */
uint16_t net_generate_checksum(net_checksum_type checksum_type, uint8_t *data, uint8_t data_length);

/**
 * @brief Generates a checksum on some data, and then compares it against the expected checksum. If the type of checksum
 *        being used is able to correct errors in the data, it does so.
 * @param checksum_type: The type of checksum to use.
 * @param expected_checksum: The data's expected checksum.
 * @param data: A pointer to the start of the data.
 * @param data_length: The number of bytes of data.
 * @returns 'true' if there are no errors in the data, or any errors were fixed; 'false' if there are uncorrected errors
 *          remaining in the data.
 */
bool net_fix_checksum_errors(net_checksum_type checksum_type, uint16_t expected_checksum, uint8_t *data, uint8_t data_length);
