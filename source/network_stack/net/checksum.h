#pragma once

#include <stdint.h>

// The type of checksum generated on a network packet.
typedef enum {
    NET_CHECKSUM_NONE = 0b00,
    NET_CHECKSUM_EVEN_PARITY = 0b01,
} net_checksum_type;

// Generates a given checksum type on some data.
uint16_t net_generate_checksum(net_checksum_type checksum_type, uint8_t *data, uint8_t data_length);
