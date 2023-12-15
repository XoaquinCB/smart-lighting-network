#include "network_stack/net.h"
#include "network_stack/dll.h"
#include "checksum.h"
#include "definitions.h"
#include <stddef.h>

// This device's network address.
#define NET_OWN_ADDRESS ((net_address) 0x54)

// The default checksum type to use on outgoing packets.
#define NET_TX_CHECKSUM_TYPE (NET_CHECKSUM_EVEN_PARITY)

// The callback to call when a NET data packet is received.
static net_receive_callback receive_callback = NULL;

// DLL always gives us the same transmit buffer and doesn't keep track of whether it's occupied,
// therefore we must keep track of that.
static bool is_buffer_occupied = false;

static void net_packet_received(uint8_t sender_address, uint8_t *data, uint8_t length);
static uint8_t net_get_next_hop(net_address destination);

void net_initialise() {
    dll_set_callback(net_packet_received);

    // TODO: other initialisation...
}

void net_update() {
    // TODO: implement this function
}

net_address net_get_own_address() {
    return NET_OWN_ADDRESS;
}

bool net_is_device_online(net_address address) {
    // TODO: implement this function
    return false;
}

void net_set_receive_callback(net_receive_callback callback) {
    receive_callback = callback;
}

uint8_t *net_create_data_buffer(uint8_t payload_buffer_length) {
    if (payload_buffer_length > 121 || is_buffer_occupied) {
        return NULL;
    }
    // Calculate the total NET packet length:
    uint8_t header_size = DATA_PACKET_FIELD_PAYLOAD_START;
    uint8_t checksum_size = 2;
    uint8_t packet_length = payload_buffer_length + header_size + checksum_size;

    // Ask DLL layer to allocate a buffer:
    uint8_t *packet = dll_create_data_buffer(packet_length);
    if (packet == NULL) {
        return NULL;
    }
    is_buffer_occupied = true;

    // Save the length of the payload to the header:
    packet[DATA_PACKET_FIELD_PAYLOAD_LENGTH] = payload_buffer_length;

    // Get pointer to the payload and return it:
    uint8_t *payload = packet + DATA_PACKET_FIELD_PAYLOAD_START;
    return payload;
}

void net_send_data_buffer(uint8_t *payload_buffer, net_address destination) {
    // Get pointer to start of packet:
    uint8_t *packet = payload_buffer - DATA_PACKET_FIELD_PAYLOAD_START;

    // Write the packet's header:
    packet[DATA_PACKET_FIELD_CONTROL_L] = 0;
    packet[DATA_PACKET_FIELD_CONTROL_H] = (NET_DATA_PACKET << 4) | (NET_TX_CHECKSUM_TYPE << 2);
    packet[DATA_PACKET_FIELD_SOURCE] = net_get_own_address();
    packet[DATA_PACKET_FIELD_DESTINATION] = destination;

    // Calculate the total NET packet length:
    uint8_t header_size = DATA_PACKET_FIELD_PAYLOAD_START;
    uint8_t payload_size = packet[DATA_PACKET_FIELD_PAYLOAD_LENGTH];
    uint8_t checksum_field_size = 2;
    uint8_t packet_size = header_size + payload_size + checksum_field_size;

    // Generate checksum on the header:
    uint16_t checksum = net_generate_checksum(NET_TX_CHECKSUM_TYPE, packet, header_size);

    // Append checksum to the end of the packet:
    uint8_t payload_length = packet[DATA_PACKET_FIELD_PAYLOAD_LENGTH];
    uint8_t checksum_field_offset_l = DATA_PACKET_FIELD_PAYLOAD_START + payload_length;
    uint8_t checksum_field_offset_h = DATA_PACKET_FIELD_PAYLOAD_START + payload_length + 1;
    packet[checksum_field_offset_l] = checksum & 0x00FF;
    packet[checksum_field_offset_h] = (checksum & 0xFF00) >> 8;

    // Send the packet to the next hop:
    uint8_t next_hop = net_get_next_hop(destination);
    dll_send_packet(next_hop, packet_size);

    // uart_put_string("\n\rNet packet transmitted\n\r  Next hop address: 0x");
    // uart_print_hex_8(next_hop);
    // uart_put_string("\n\r  Packet type: 0x");
    // uart_print_hex_8(NET_DATA_PACKET);
    // uart_put_string("\n\r  Checksum type: 0x");
    // uart_print_hex_8(NET_TX_CHECKSUM_TYPE);
    // uart_put_string("\n\r  Payload size: 0x");
    // uart_print_hex_8(payload_size);
    // uart_put_string("\n\r  Payload data: ");
    // for (uint8_t i = 0; i < payload_size; i++) {
    //     uart_print_hex_8(payload_buffer[i]);
    //     uart_put_byte(' ');
    // }
    // uart_put_string("\n\r  Packet data: ");
    // for (uint8_t i = 0; i < packet_size; i++) {
    //     uart_print_hex_8(packet[i]);
    //     uart_put_byte(' ');
    // }
    // uart_put_string("\n\r");

    is_buffer_occupied = false;
}

void net_discard_data_buffer(uint8_t *payload_buffer) {
    if (payload_buffer == NULL) {
        return;
    }
    is_buffer_occupied = false;
}

static void net_packet_received(uint8_t sender_address, uint8_t *data, uint8_t length) {
    // uart_put_string("\n\rNET PACKET RECEIVED\n\r");
    // net_packet_type packet_type = (data[DATA_PACKET_FIELD_CONTROL_H] & 0xF0) >> 4;
    // net_checksum_type checksum_type = (data[DATA_PACKET_FIELD_CONTROL_H] & 0x0C) >> 2;
    // uart_put_string("\n\rNet packet received\n\r  Packet data: 0x");
    // for (uint8_t i = 0; i < length; i++) {
    //     uart_print_hex_8(data[i]);
    //     uart_put_byte(' ');
    // }
    // uart_put_string("\n\r  Previous hop address: 0x");
    // uart_print_hex_8(sender_address);
    // uart_put_string("\n\r  Packet type: 0x");
    // uart_print_hex_8(packet_type);
    // uart_put_string("\n\r  Checksum type: 0x");
    // uart_print_hex_8(checksum_type);
    // uart_put_string("\n\r  Payload length: 0x");
    // uint8_t payload_length = length - DATA_PACKET_FIELD_PAYLOAD_START - 2;
    // uart_print_hex_8(payload_length);
    // uart_put_string("\n\r  Payload data: ");
    // for (uint8_t i = 0; i < payload_length; i++) {
    //     uart_print_hex_8(data[i + DATA_PACKET_FIELD_PAYLOAD_START]);
    //     uart_put_byte(' ');
    // }
    // uart_put_string("\n\r");
}

static uint8_t net_get_next_hop(net_address destination) {
    // TODO: implement this fuction
    static bool b = false;

    uint8_t address = b ? 0xA1 : 0xA2;

    b = !b;

    return address;
}