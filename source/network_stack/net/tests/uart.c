#include "uart.h"
#include <avr/io.h>

void uart_initialise() {
    // Set up UART peripheral:
    // - Baud rate = 9600
    // - Character size = 8 bits
    // - Parity = none
    // - Stop bits = one
	const int baud_rate = 9600;
	UBRR0 = (F_CPU / (baud_rate * 8L) - 1);
	UCSR0A = (1 << U2X0);
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
}

uint8_t uart_get_byte() {
    // Wait for RX buffer to contain a byte:
	while (!(UCSR0A & (1 << RXC0)));

    // Return byte from RX buffer:
	return UDR0;
}

void uart_put_byte(uint8_t byte) {
    // Wait for TX buffer to be empty:
	while (!(UCSR0A & (1 << UDRE0)));

    // Write byte to TX buffer:
	UDR0 = byte;
}

void uart_put_string(const char *string) {
	for (unsigned i = 0; string[i] != '\0'; i++) {
        uart_put_byte(string[i]);
    }
}

int uart_get_byte_nonblocking() {
	if (UCSR0A & (1 << RXC0)) {
        // If RX buffer contains data, return it:
		return UDR0;
	} else {
        // Otherwise, return -1:
		return -1;
	}
}

void uart_print_hex_8(uint8_t value) {
    uint8_t high_nibble = (value & 0xF0) >> 4;
    uint8_t low_nibble = value & 0x0F;
    char high_nibble_char = (high_nibble < 10) ? high_nibble + '0' : high_nibble - 10 + 'A';
    char low_nibble_char = (low_nibble < 10) ? low_nibble + '0' : low_nibble - 10 + 'A';
    uart_put_byte(high_nibble_char);
    uart_put_byte(low_nibble_char);
}

void uart_print_hex_16(uint16_t value) {
    uart_print_hex_8((value & 0xFF00) >> 8);
    uart_put_byte('-');
    uart_print_hex_8((value & 0x00FF));
}
