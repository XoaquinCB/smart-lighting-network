#pragma once

#include <stdint.h>

// DLL address
typedef uint8_t dll_address;

// Address which broadcasts to all nodes
#define DLL_BROADCAST_ADDRESS ((dll_address) 0xFF)

// typedef uint8_t BYTE;
typedef void (*dll_callback)(dll_address sender_address, uint8_t *data, uint8_t length);
// Defines the type dll_callback, which is a pointer to a function with these parameters, that returns void

typedef enum {
    DLL_TRANSMISSION_SUCCESS,
    DLL_NODE_UNREACHABLE,
    DLL_PACKET_TOO_BIG,
} dll_send_response;

//PUBLIC FUNCTIONS
// Returns a pointer to the starting memory address to put packet data into
// If requested size if too big (>128), returns NULL
uint8_t *dll_create_data_buffer(uint8_t net_packet_length);

// Sends data in the packet buffer to a given address (or broadcast for address 0xFF)
dll_send_response dll_send_packet(dll_address destination_address, uint8_t packet_length);

// Sets the NET layer function to be called when a frame is received
// Pass a pointer to the function that should be called
void dll_set_callback(dll_callback callback);

// Update function to be repeatedly
void dll_update();