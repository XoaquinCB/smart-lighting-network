#include "dll_private.h"
#include "network_stack/phy.h"
#include "uart.h"
#include <avr/interrupt.h>

void example_net_callback_function(uint8_t sender_address, uint8_t *data, uint8_t length);

int main(void) {
    phy_initialise();
    sei();
    init_uart0();
    uint8_t x;
    for(x=0; x<20; x++) {
        put_ch('\n');
    }
    put_str("-------------------------------------------------------------");
    put_ch('\n');
    put_ch('\n');

    dll_set_callback(&example_net_callback_function);
    
    // Fill packet_buffer with example data
    uint8_t packet_size = 120;
    uint8_t *packet_ptr = dll_create_data_buffer(packet_size);
    uint8_t i;
    for(i=0; i<packet_size; i++) {
        packet_ptr[i] = i + 111;
    }
    uint8_t dest_address = NODE_HARDWARE_ADDRESS;

    //put_str("Test of packet fragmentation and transmission:\n");
    //put_str("NET packet to be transmitted: ");
    //print_buffer(packet_ptr, packet_size);

    dll_send_response result = dll_send_packet(dest_address, packet_size);
    //put_str("\n\nFunction returned with result: ");
    //print_int(result);

    while(1) {
        dll_check_for_transmission();
    }
}

void example_net_callback_function(uint8_t sender_address, uint8_t *data, uint8_t length) {
    put_str("\n\nNET callback function run.\nPacket origin address: ");
    put_hex(sender_address);
    put_str("\n\nNET packet received: ");
    print_buffer(data, length);
}