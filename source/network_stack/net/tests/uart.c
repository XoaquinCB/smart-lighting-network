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
