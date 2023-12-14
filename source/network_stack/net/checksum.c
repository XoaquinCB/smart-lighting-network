#include "checksum.h"

uint16_t net_generate_checksum(net_checksum_type checksum_type, uint8_t *data, uint8_t length) {
    switch (checksum_type) {

        case NET_CHECKSUM_EVEN_PARITY: {
            uint8_t parity_bit = 0;
            for (uint8_t byte_i = 0; byte_i != length; byte_i++) {
                uint8_t current_byte = data[byte_i];
                for (uint8_t bit_i = 0; bit_i < 8; bit_i++) {
                    parity_bit ^= (current_byte & 0x01);
                    current_byte >>= 1;
                }
            }
            return (uint16_t) parity_bit;
        } break;

        case NET_CHECKSUM_NONE:
        default:
            return 0;

    }
}
