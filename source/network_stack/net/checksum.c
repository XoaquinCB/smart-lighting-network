#include "checksum.h"

uint16_t net_generate_checksum(net_checksum_type checksum_type, uint8_t *data, uint8_t length) {
    switch (checksum_type) {
        case NET_CHECKSUM_EVEN_PARITY: {
            uint8_t parity_bit = 0;
            // For each byte in the data:
            for (uint8_t byte_i = 0; byte_i != length; byte_i++) {
                uint8_t current_byte = data[byte_i];
                // For each bit in the byte:
                for (uint8_t bit_i = 0; bit_i < 8; bit_i++) {
                    // Update the parity bit:
                    parity_bit ^= (current_byte & 0x01);

                    // Move to the next bit:
                    current_byte >>= 1;
                }
            }
            // Return the parity bit in the least-significant bit of the checksum:
            return (uint16_t) parity_bit;
        } break;

        case NET_CHECKSUM_NONE:
        default:
            // No checksum to generate. Checksum should be zero:
            return 0;
    }
}

bool net_fix_checksum_errors(net_checksum_type checksum_type, uint16_t checksum, uint8_t *data, uint8_t data_length) {
    // Currently there are no checksum types that actually fix any errors, but there could be in the future.

    switch (checksum_type) {
        case NET_CHECKSUM_EVEN_PARITY: {
            // Generate checksum and compare against expected checksum:
            uint16_t generated_checksum = net_generate_checksum(NET_CHECKSUM_EVEN_PARITY, data, data_length);
            return generated_checksum == checksum;
        } break;

        case NET_CHECKSUM_NONE:
            // No checksum to generate; checksum check should always pass:
            return true;

        default:
            // Unknown checksum type; checksum check should pass automatically:
            return true;
    }
}
