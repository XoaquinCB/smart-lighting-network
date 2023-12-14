#pragma once

#include <stdint.h>

// Initialises the UART library.
void uart_initialise();

// Blocks until a byte has been received and the returns it.
uint8_t uart_get_byte();

// Transmits a byte.
void uart_put_byte(uint8_t byte);

// Transmits a null-terminated string.
void uart_put_string(const char *str);

// Returns a byte if it has been received, or -1 if not.
int uart_get_byte_nonblocking();
