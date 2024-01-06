#include "network_stack/dll.h"
#include "network_stack/net.h"
#include "../packets.h"
#include "../routing.h"
#include "uart.h"
#include <stdint.h>
#include <stddef.h>


void data_packet_receive_callback(net_address source, uint8_t *payload, uint8_t length) {
    uart_put_string("Data packet received:\n\r  Source address: ");
    uart_print_hex_8(source);
    uart_put_string("\n\r  Data:           ");
    for (uint8_t i = 0; i < length; i++) {
        uart_print_hex_8(payload[i]);
        uart_put_byte(' ');
    }
    uart_put_string("\n\r");
}

int main() {
    uart_initialise();
    uart_put_string("\n\r============================================================\n\r");
    uart_put_string("--------------------------- TX tests -----------------------\n\r");


    // Write and send a data packet:
    uart_put_string("\n\r--- Sending data packet ---\n\r");
    uint8_t *buffer = net_get_data_buffer();
    uint8_t data_length = 10;
    net_address destination = 0x02;
    for (uint8_t i = 0; i < data_length; i++) {
        buffer[i] = i;
    }
    net_send_data_packet(destination, data_length);

    // Broadcast a ping request packet:
    uart_put_string("\n\r--- Broadcasting ping_request packet ---\n\r");
    net_send_ping_request_packet(DLL_BROADCAST_ADDRESS);

    // Send a ping response packet:
    uart_put_string("\n\r--- Sending ping response packet ---\n\r");
    dll_address neighbouring_node = 0x12;
    net_send_ping_response_packet(neighbouring_node);

    // Send a link state packet:
    uart_put_string("\n\r--- Flooding link state packet ---\n\r");
    net_send_link_state_packet();


    uart_put_string("--------------------------- RX tests -----------------------\n\r");

    net_set_receive_callback(data_packet_receive_callback);

    // Receive data packet destined to our address:
    uart_put_string("\n\r--- Receiving data packet to our address ---\n\r");
    uint8_t data_packet_1[] = { 0x00, 0x04, 0x07, 0x01, 0x05, 0x00, 0x01, 0x02, 0x03, 0x04, 0x01, 0x00 };
    net_handle_received_packet(0x12, data_packet_1, sizeof(data_packet_1));

    // Receive data packet destined to our address with parity error:
    uart_put_string("\n\r--- Receiving data packet to our address with parity error ---\n\r");
    uint8_t data_packet_2[] = { 0x00, 0x04, 0x07, 0x01, 0x05, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x00 };
    net_handle_received_packet(0x12, data_packet_2, sizeof(data_packet_2));

    // Receive data packet destined to our address with length error:
    uart_put_string("\n\r--- Receiving data packet to our address with length error ---\n\r");
    uint8_t data_packet_3[] = { 0x00, 0x04, 0x07, 0x01, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x00 };
    net_handle_received_packet(0x12, data_packet_3, sizeof(data_packet_3));

    // Receve data packet destined to different address:
    uart_put_string("\n\r--- Receiving data packet to different address ---\n\r");
    uint8_t data_packet_4[] = { 0x00, 0x04, 0x07, 0x03, 0x05, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x00 };
    net_handle_received_packet(0x12, data_packet_4, sizeof(data_packet_4));

    // Receve data packet destined to different address with parity error:
    uart_put_string("\n\r--- Receiving data packet to different address with parity error ---\n\r");
    uint8_t data_packet_5[] = { 0x00, 0x04, 0x07, 0x03, 0x05, 0x00, 0x01, 0x02, 0x03, 0x04, 0x01, 0x00 };
    net_handle_received_packet(0x12, data_packet_5, sizeof(data_packet_5));

    // Receve data packet destined to different address with length error:
    uart_put_string("\n\r--- Receiving data packet to different address with length error ---\n\r");
    uint8_t data_packet_6[] = { 0x00, 0x04, 0x07, 0x03, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x01, 0x00 };
    net_handle_received_packet(0x12, data_packet_6, sizeof(data_packet_6));

    // Receive ping request:
    uart_put_string("\n\r--- Receiving ping request packet ---\n\r");
    uint8_t ping_request_packet_1[] = { 0x00, 0x24, 0x00, 0x00 };
    net_handle_received_packet(0x12, ping_request_packet_1, sizeof(ping_request_packet_1));

    // Receive ping request with parity error:
    uart_put_string("\n\r--- Receiving ping request packet with parity error ---\n\r");
    uint8_t ping_request_packet_2[] = { 0x00, 0x24, 0x01, 0x00 };
    net_handle_received_packet(0x12, ping_request_packet_2, sizeof(ping_request_packet_2));

    // Receive ping request with length error:
    uart_put_string("\n\r--- Receiving ping request packet with length error ---\n\r");
    uint8_t ping_request_packet_3[] = { 0x00, 0x24, 0x00, 0x00, 0x00 };
    net_handle_received_packet(0x12, ping_request_packet_3, sizeof(ping_request_packet_3));

    // Receive ping response:
    uart_put_string("\n\r--- Receiving ping response packet ---\n\r");
    uint8_t ping_response_packet_1[] = { 0x00, 0x34, 0x04, 0x00, 0x00 };
    net_handle_received_packet(0x14, ping_response_packet_1, sizeof(ping_response_packet_1));

    // Receive ping response with parity error:
    uart_put_string("\n\r--- Receiving ping response packet with parity error ---\n\r");
    uint8_t ping_response_packet_2[] = { 0x00, 0x34, 0x04, 0x01, 0x00 };
    net_handle_received_packet(0x14, ping_response_packet_2, sizeof(ping_response_packet_2));

    // Receive ping response with length error:
    uart_put_string("\n\r--- Receiving ping response packet with length error ---\n\r");
    uint8_t ping_response_packet_3[] = { 0x00, 0x34, 0x04, 0x00, 0x00, 0x00 };
    net_handle_received_packet(0x14, ping_response_packet_3, sizeof(ping_response_packet_3));

    // Receive link state packet from different address:
    uart_put_string("\n\r--- Receiving link state packet from different address ---\n\r");
    uint8_t link_state_packet_1[] = { 0x00, 0x14, 0x02, 0x00, 0x03, 0x01, 0x02, 0x03, 0x01, 0x00 };
    net_handle_received_packet(0x12, link_state_packet_1, sizeof(link_state_packet_1));

    // Receive link state packet from different address with parity error:
    uart_put_string("\n\r--- Receiving link state packet from different address with parity error ---\n\r");
    uint8_t link_state_packet_2[] = { 0x00, 0x14, 0x02, 0x00, 0x03, 0x01, 0x02, 0x03, 0x00, 0x00 };
    net_handle_received_packet(0x12, link_state_packet_2, sizeof(link_state_packet_2));

    // Receive link state packet from different address with length error:
    uart_put_string("\n\r--- Receiving link state packet from different address with length error ---\n\r");
    uint8_t link_state_packet_3[] = { 0x00, 0x14, 0x02, 0x00, 0x05, 0x01, 0x02, 0x03, 0x01, 0x00 };
    net_handle_received_packet(0x12, link_state_packet_3, sizeof(link_state_packet_3));

    // Receive link state packet from our address:
    uart_put_string("\n\r--- Receiving link state packet from our address ---\n\r");
    uint8_t link_state_packet_4[] = { 0x00, 0x14, 0x01, 0x00, 0x03, 0x01, 0x02, 0x03, 0x01, 0x00 };
    net_handle_received_packet(0x12, link_state_packet_4, sizeof(link_state_packet_4));

    // Receive link state packet from our address with parity error:
    uart_put_string("\n\r--- Receiving link state packet from our address with parity error ---\n\r");
    uint8_t link_state_packet_5[] = { 0x00, 0x14, 0x01, 0x00, 0x03, 0x01, 0x02, 0x03, 0x00, 0x00 };
    net_handle_received_packet(0x12, link_state_packet_5, sizeof(link_state_packet_5));

    // Receive link state packet from our address with length error:
    uart_put_string("\n\r--- Receiving link state packet from our address with length error ---\n\r");
    uint8_t link_state_packet_6[] = { 0x00, 0x14, 0x01, 0x00, 0x05, 0x01, 0x02, 0x03, 0x01, 0x00 };
    net_handle_received_packet(0x12, link_state_packet_6, sizeof(link_state_packet_6));
}

//*************************** dll.h emulated implementation *************************//

uint8_t dll_tx_buffer[128];

uint8_t *dll_create_data_buffer(uint8_t net_packet_length) {
    if (net_packet_length <= 128) {
        return dll_tx_buffer;
    } else {
        return NULL;
    }
}

dll_send_response dll_send_packet(uint8_t destination_address, uint8_t packet_length) {
    // Print out the packet's physical destination and payload contents:
    uart_put_string("Sending DLL packet:\n\r  Next hop: ");
    uart_print_hex_8(destination_address);
    uart_put_string("\n\r  Payload:  ");
    for (uint8_t i = 0; i < packet_length; i++) {
        uart_print_hex_8(dll_tx_buffer[i]);
        uart_put_byte(' ');
    }
    uart_put_string("\n\r");
    return DLL_TRANSMISSION_SUCCESS;
}

//*************************** routing.h emulated implementation *************************//

dll_address net_get_next_hop(net_address destination) {
    // Just return a static address for emulation purposes.
    return 0x50;
}

bool net_are_nodes_linked(net_address node_1, net_address node_2) {
    if (node_1 != net_get_own_address() || node_2 > NET_MAX_ADDRESS) {
        return false;
    }

    // Emulate that our node is linked to nodes with the following logical addresses:
    bool is_linked = (node_2 == 0x02 || node_2 == 0x03 || node_2 == 0x07);
    return is_linked;
}

bool net_is_node_neighbour(dll_address node) {
    // Emulate that we have neighbours with the following physical addresses:
    bool is_neighbour = (node == 0x12 || node == 0x13 || node == 0x17);
    return is_neighbour;
}

void net_notify_ping_response(dll_address physical_address, net_address logical_address) {
    uart_put_string("Ping response received:\n\r  Physical address: ");
    uart_print_hex_8(physical_address);
    uart_put_string("\n\r  Logical address:  ");
    uart_print_hex_8(logical_address);
    uart_put_string("\n\r");
}

bool net_notify_link_state_packet(net_address source, uint8_t sequence_number, const net_address *node_list, uint8_t node_count) {
    bool is_valid = (source != net_get_own_address());
    if (is_valid == false) {
        return false;
    }

    uart_put_string("Link state packet received:\n\r  Source address:  ");
    uart_print_hex_8(source);
    uart_put_string("\n\r  Sequence number: ");
    uart_print_hex_8(sequence_number);
    uart_put_string("\n\r  Nodes:           ");
    for (uint8_t i = 0; i < node_count; i++) {
        uart_print_hex_8(node_list[i]);
        uart_put_string(", ");
    }
    uart_put_string("\n\r");

    return true;
}

//*************************** net.h emulated implementation *************************//

net_address net_get_own_address() {
    // Use 0x01 as our own address:
    return 0x01;
}
