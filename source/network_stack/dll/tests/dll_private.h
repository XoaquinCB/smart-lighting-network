#include "network_stack/dll.h"

#define FLAG_BYTE 0x7E // 01111110
#define ESCAPE_BYTE 0x7D // 01111101
#define NODE_HARDWARE_ADDRESS 0xA1

//Frame Buffer Field Addresses
#define FRAME_HEADER_FIELD 0x00
#define FRAME_CONTROL_FIELD 0x01 // And 0x02
#define FRAME_ADDRESS_FIELD 0x03 // And 0x04
#define FRAME_LENGTH_FIELD 0x05
#define FRAME_DATA_FIELD 0x06 // Up to 0x1D (23)

#define BUFSIZE 128
#define FRAMEBUFSIZE 59
//Maximum frame size is 32 bytes, but need more to account for byte stuffing
//Neither control bytes will ever equal the flag/escape byte
//Both address bytes could equal the flag/escape byte
//The length byte will never equal the flag/escape byte
//Any NET packet byte could equal the flag/escape byte
//Both checksum bytes could equal the flag/escape byte'
//Therefore need 2 + 23 + 2 = 27 additional bytes (32 + 27 = 59)
#define CHECKSUM_MODE 1
//1: Even parity bit
//2: Odd parity bit
//3: 8-Bit Checksum
//4: CRC-16 (Polynomial 0x8005)
//5: CRC-16-CCITT (Polynomial 0x1021)
//8: Hamming Code

typedef enum {
    ERROR_EVEN_PARITY = 1,
    ERROR_ODD_PARITY = 2,
} error_checking_types;

typedef enum {
    MORE_FRAMES_EXPECTED,
    FINAL_FRAME,
    CONTROL_FRAME,
    ADDRESS_MISMATCH,
} frame_receive_process_responses;

typedef enum {
    CONTROL_ACK = 0b00000001,
    CONTROL_RTC = 0b00000010,
    CONTROL_CTC = 0b00000011,
    CONTROL_BUSY =0b00000100,
} control_frame_types;

//FRAME CONSTRUCTION FUNCTIONS
uint8_t byte_stuff_frame(uint8_t length);
uint8_t byte_unstuff_frame(uint8_t length);
uint8_t prepare_control_frame(uint8_t address, control_frame_types type);

//FLOW CONTROL FUNCTIONS
uint8_t establish_connection(uint8_t address);
uint8_t transmit_frame(uint8_t length);
uint8_t mimic_transmit_frame(uint8_t length);

//RECEIVER FUNCTIONS
void dll_check_for_transmission();
void receive_frame(uint8_t length);
frame_receive_process_responses process_received_frame(uint8_t length); //Validates data with checksum, and sends acknowledgment if all is okay

//CHECKSUM FUNCTIONS
uint16_t checksum_parity(uint8_t *ptr, uint8_t length, uint8_t type);
uint16_t checksum_8_bit(uint8_t *ptr, uint8_t length);
uint16_t checksum_CRC16(uint8_t *ptr, uint8_t length);
uint16_t checksum_CRC16_CCITT(uint8_t *ptr, uint8_t length);
int byte_parity(uint8_t byte);

