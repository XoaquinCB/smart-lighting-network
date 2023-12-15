#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef UART_H
#define UART_H
#define F_CPU 12000000

//uart
void init_uart0 (void);
int8_t get_ch_non_block (void);
int8_t get_ch_block (void);
void put_ch (char ch);
void put_str (char *str);
void get_str(char* buf, unsigned int size);

//DEBUGGING FUNCTIONS
void print_buffer(uint8_t *bufptr, uint8_t size);
void print_byte_in_binary(uint8_t byte);
void put_hex(uint8_t byte);
void print_int(uint8_t i);
uint8_t check_uart();
uint8_t *get_uart_buffer();
uint8_t get_byte_count();

#endif
