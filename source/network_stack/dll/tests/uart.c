#include "uart.h"

uint8_t uart_buffer[59]; //Max frame size
uint8_t byte_count;
volatile uint8_t uart_transmission_complete = 0;

void init_uart0 (void)
{
	/* Configure 9600 baud , 8-bit , no parity and one stop bit */
	const int baud_rate = 9600;
	UBRR0H = (F_CPU/(baud_rate*16L)-1) >> 8;
	UBRR0L = (F_CPU/(baud_rate*16L)-1);
	UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
	UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

int8_t get_ch_non_block (void)
{
	if(UCSR0A & _BV(RXC0))
		return UDR0;
	else
		return -1;
}

int8_t get_ch_block (void)
{
	while(!(UCSR0A & _BV(RXC0)));
	return UDR0;
}

void put_ch ( char ch)
{
	while (!(UCSR0A & _BV(UDRE0)));
	UDR0 = ch;
}

void get_str(char* buf, unsigned int size){
	unsigned int i;
	int8_t ch;
	for(i=0; (ch=get_ch_block()) > 0 && i<size; i++)
		buf[i] = ch;
}

void put_str (char *str)
{
	int i;
	for (i=0; str[i]; i++) put_ch(str[i]);
}

void print_buffer(uint8_t *bufptr, uint8_t size) {
    int i;
    for(i=0; i<size; i++) {
		put_ch('0');
		put_ch('x');
		put_hex(bufptr[i]);
		put_ch(',');
    }
}

void print_byte_in_binary(uint8_t byte) {
    int i;
    for (i=7; 0 <= i; i--) {
        //printf("%c", (byte & (1 << i)) ? '1' : '0');
    }
}

void put_hex(uint8_t byte) {
    uint8_t first_value = (byte & 0xF0) >> 4;
    uint8_t second_value = byte & 0x0F;

	if(first_value < 10) { // Just need to add hex value of "0"
		first_value += '0';
	} else { // Otherwise we use A-F chars
		first_value += 'A' - 10;
	}

	if(second_value < 10) { // Just need to add hex value of "0"
		second_value += '0';
	} else { // Otherwise we use A-F chars
		second_value += 'A' - 10;
	}

    put_ch(first_value);
    put_ch(second_value);
}

void print_int(uint8_t i) {
	char string[4];
	itoa(i, string, 10);
	put_str(string);
}

ISR(USART0_RX_vect) {
	uint8_t byte = get_ch_non_block();
	//put_ch(byte);
	if(byte == 0x0D) {//0x0D is carriage return, use NULL terminator for Il Mattos
		uart_transmission_complete = 1;
	} else {
		uart_buffer[byte_count] = byte;
		byte_count++;
	}
}

uint8_t check_uart() {
	//put_str("Called-----------------------");
	return uart_transmission_complete;
}

uint8_t *get_uart_buffer() {
	return uart_buffer;
}

uint8_t get_byte_count() {
	uint8_t temp = byte_count;
	byte_count = 0;
	uart_transmission_complete = 0;
	return temp;
}