#include "../checksum.h"
#include "uart.h"
#include <stdint.h>

void test_checksum(uint8_t *data, uint8_t data_length, net_checksum_type checksum_type, uint16_t expected_checksum);
void print_hex_array(const uint8_t *data, uint8_t length);

int main() {
    uart_initialise();

    // Wait until the user has pressed ENTER:
    int rx_byte = 0;
    do {
        rx_byte = uart_get_byte_nonblocking();
    }
    while (rx_byte != '\n' && rx_byte != '\r');

    uart_put_string("Starting test.\n\r\n\r");

    // ############################################################################################
    // Carry out the tests...

    test_checksum(
        (uint8_t[]) { 0b00000001 }, 1,
        NET_CHECKSUM_NONE,
        0
    );

    test_checksum(
        (uint8_t[]) { 0b00000000 }, 1,
        NET_CHECKSUM_NONE,
        0
    );

    test_checksum(
        (uint8_t[]) { 0b00000001 }, 1,
        NET_CHECKSUM_EVEN_PARITY,
        1
    );

    test_checksum(
        (uint8_t[]) { 0b00000000 }, 1,
        NET_CHECKSUM_EVEN_PARITY,
        0
    );

    test_checksum(
        (uint8_t[]) { 0b10100101, 0b00010111, 0b11010100, 0b11111110, 0b11111111, 0b00000000 }, 6,
        NET_CHECKSUM_EVEN_PARITY,
        1
    );

    test_checksum(
        (uint8_t[]) { 0b00000001, 0b10111101, 0b00010001, 0b11001101 }, 5,
        NET_CHECKSUM_EVEN_PARITY,
        0
    );

    // ############################################################################################

    uart_put_string("\n\rFinished.\n\r\n\r");
}

void test_checksum(uint8_t *data, uint8_t data_length, net_checksum_type checksum_type, uint16_t expected_checksum) {
    // Calculate the checksum:
    uint16_t calculated_checksum = net_generate_checksum(checksum_type, data, data_length);

    // Print whether the test was successful:
    uart_put_string("\n\rTest result: ");
    if (calculated_checksum == expected_checksum) {
        uart_put_string("PASS");
    } else {
        uart_put_string("FAIL");
    }

    // Print the data that was tested:
    uart_put_string("\n\r  Data: ");
    print_hex_array(data, data_length);

    // Print the checksum type used:
    uart_put_string("\n\r  Checksum type: ");
    switch (checksum_type) {
        case NET_CHECKSUM_NONE: {
            uart_put_string("No checksum");
        } break;
        case NET_CHECKSUM_EVEN_PARITY: {
            uart_put_string("Even parity");
        } break;
    }

    // Print the expected and calcualted checksums:
    uart_put_string("\n\r  Expected checksum: ");
    uart_print_hex_16(expected_checksum);
    uart_put_string("\n\r  Calculated checksum: ");
    uart_print_hex_16(calculated_checksum);
    uart_put_string("\n\r");
}

void print_hex_array(const uint8_t *data, uint8_t length) {
    for (uint8_t i = 0; i != length; i++) {
        uart_print_hex_8(data[i]);
        uart_put_byte(' ');
    }
}