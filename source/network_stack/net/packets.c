#include "checksum.h"
#include "packets.h"
#include "routing.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    NET_DATA_PACKET = 0b0000,
    NET_LINK_STATE_PACKET = 0b0001,
    NET_PING_REQUEST_PACKET = 0b0010,
    NET_PING_RESPONSE_PACKET = 0b0011,
} net_packet_type;

enum generic_packet_fields {
    PACKET_FIELD_CONTROL_L = 0,
    PACKET_FIELD_CONTROL_H = 1,
};

enum data_packet_fields {
    DATA_PACKET_FIELD_CONTROL_L = 0,
    DATA_PACKET_FIELD_CONTROL_H = 1,
    DATA_PACKET_FIELD_SOURCE_ADDRESS = 2,
    DATA_PACKET_FIELD_DESTINATION_ADDRESS = 3,
    DATA_PACKET_FIELD_PAYLOAD_LENGTH = 4,
    DATA_PACKET_FIELD_PAYLOAD_START = 5,
};

enum link_state_packet_fields {
    LINK_STATE_PACKET_FIELD_CONTROL_L = 0,
    LINK_STATE_PACKET_FIELD_CONTROL_H = 1,
    LINK_STATE_PACKET_FIELD_SOURCE_ADDRESS = 2,
    LINK_STATE_PACKET_FIELD_SEQUENCE_NUMBER = 3,
    LINK_STATE_PACKET_FIELD_NODE_COUNT = 4,
    LINK_STATE_PACKET_FIELD_NODE_LIST_START = 5,
};

enum ping_request_packet_fields {
    PING_REQUEST_PACKET_FIELD_CONTROL_L = 0,
    PING_REQUEST_PACKET_FIELD_CONTROL_H = 1,
};

enum ping_response_packet_fields {
    PING_RESPONSE_PACKET_FIELD_CONTROL_L = 0,
    PING_RESPONSE_PACKET_FIELD_CONTROL_H = 1,
    PING_RESPONSE_PACKET_FIELD_SOURCE_ADDRESS = 2,
};

// The callback to call when a network data packet is received.
static net_receive_callback receive_callback = NULL;

uint8_t *net_get_data_buffer() {
    // uint8_t *packet = dll_get_data_buffer();
    uint8_t *packet = dll_create_data_buffer(0);
    return &packet[DATA_PACKET_FIELD_PAYLOAD_START];
}

uint8_t net_get_data_buffer_size() {
    uint8_t header_size = DATA_PACKET_FIELD_PAYLOAD_START;
    uint8_t checksum_size = 2;
    // uint8_t max_packet_size = dll_get_data_buffer_size();
    uint8_t max_packet_size = 128;
    uint8_t max_payload_size = max_packet_size - header_size - checksum_size;
    return max_payload_size;
}

void net_send_data_packet(net_address destination, uint8_t data_length) {
    if (data_length > net_get_data_buffer_size() || destination > NET_MAX_ADDRESS) {
        return;
    }

    // Resolve the next hop:
    uint8_t next_hop = net_get_next_hop(destination);
    if (next_hop == NET_NEXT_HOP_NOT_RESOLVED) {
        return;
    }

    // Get a pointer to DLL's data buffer:
    // uint8_t *packet = dll_get_data_buffer();
    uint8_t *packet = dll_create_data_buffer(0);

    // Calculate the total packet size:
    uint8_t header_size = DATA_PACKET_FIELD_PAYLOAD_START;
    uint8_t checksum_size = 2;
    uint8_t packet_size = header_size + data_length + checksum_size;

    // Write the packet's header:
    packet[DATA_PACKET_FIELD_CONTROL_L] = 0;
    packet[DATA_PACKET_FIELD_CONTROL_H] = (NET_DATA_PACKET << 4) | (NET_TX_CHECKSUM_TYPE << 2);
    packet[DATA_PACKET_FIELD_SOURCE_ADDRESS] = net_get_own_address();
    packet[DATA_PACKET_FIELD_DESTINATION_ADDRESS] = destination;
    packet[DATA_PACKET_FIELD_PAYLOAD_LENGTH] = data_length;

    // Generate checksum on the header:
    uint16_t checksum = net_generate_checksum(NET_TX_CHECKSUM_TYPE, packet, header_size);

    // Write checksum to the end of the packet:
    uint8_t checksum_field_offset_l = DATA_PACKET_FIELD_PAYLOAD_START + data_length;
    uint8_t checksum_field_offset_h = DATA_PACKET_FIELD_PAYLOAD_START + data_length + 1;
    packet[checksum_field_offset_l] = checksum & 0x00FF;
    packet[checksum_field_offset_h] = (checksum & 0xFF00) >> 8;

    // Send the packet to the next hop:
    dll_send_packet(next_hop, packet_size);
}

void net_send_link_state_packet() {
    // Get a pointer to DLL's data buffer:
    // uint8_t *packet = dll_get_data_buffer();
    uint8_t *packet = dll_create_data_buffer(0);

    static uint8_t sequence_number = 0;
    sequence_number++;

    // Write the packet's header:
    packet[LINK_STATE_PACKET_FIELD_CONTROL_L] = 0;
    packet[LINK_STATE_PACKET_FIELD_CONTROL_H] = (NET_LINK_STATE_PACKET << 4) | (NET_TX_CHECKSUM_TYPE << 2);
    packet[LINK_STATE_PACKET_FIELD_SOURCE_ADDRESS] = net_get_own_address();
    packet[LINK_STATE_PACKET_FIELD_SEQUENCE_NUMBER] = sequence_number;

    uint8_t *node_list = &packet[LINK_STATE_PACKET_FIELD_NODE_LIST_START];
    const uint8_t max_node_count = 121;
    uint8_t node_count = 0;

    // Write all neighbouring nodes to the packet:
    net_address own_address = net_get_own_address();
    for (net_address node = 0x00; node != NET_MAX_ADDRESS + 1; node++) {
        if (node_count >= max_node_count) {
            // Maximum payload size has been reached; don't add any more nodes to the list.
            break;
        }

        // If the node is linked, add it to the list:
        bool is_linked = net_are_nodes_linked(own_address, node);
        if (is_linked) {
            node_list[node_count++] = node;
        }
    }

    // Write the node count to the packet's header:
    packet[LINK_STATE_PACKET_FIELD_NODE_COUNT] = node_count;

    // Generate the checksum on the packet:
    const uint8_t checksum_size = LINK_STATE_PACKET_FIELD_NODE_LIST_START + node_count;
    uint16_t checksum = net_generate_checksum(NET_TX_CHECKSUM_TYPE, packet, checksum_size);

    // Write checksum to the end of the packet:
    uint8_t checksum_field_offset_l = LINK_STATE_PACKET_FIELD_NODE_LIST_START + node_count;
    uint8_t checksum_field_offset_h = LINK_STATE_PACKET_FIELD_NODE_LIST_START + node_count + 1;
    packet[checksum_field_offset_l] = checksum & 0x00FF;
    packet[checksum_field_offset_h] = (checksum & 0xFF00) >> 8;

    // Send the packet to all neighbouring nodes:
    uint8_t packet_size = LINK_STATE_PACKET_FIELD_NODE_LIST_START + node_count + 2;
    for (dll_address address = 0; address < 0xFF; address++) {
        if (net_is_node_neighbour(address)) {
            dll_send_packet(address, packet_size);
        }
    }
}

void net_send_ping_request_packet(dll_address node) {
    const uint8_t packet_size = 4;
    // if (ping_request_packet_size > dll_get_data_buffer_size()) {
    if (packet_size > 128) {
        return;
    }

    // Get pointer to DLL data buffer:
    // uint8_t *packet = dll_get_data_buffer();
    uint8_t *packet = dll_create_data_buffer(0);

    // Write the packet's header:
    packet[PING_REQUEST_PACKET_FIELD_CONTROL_L] = 0;
    packet[PING_REQUEST_PACKET_FIELD_CONTROL_H] = (NET_PING_REQUEST_PACKET << 4) | (NET_TX_CHECKSUM_TYPE << 2);

    // Generate the checksum on the header:
    const uint8_t checksum_size = 2;
    uint16_t checksum = net_generate_checksum(NET_TX_CHECKSUM_TYPE, packet, checksum_size);

    // Write the checksum to the end of the packet:
    packet[packet_size - 2] = checksum & 0x00FF;
    packet[packet_size - 1] = (checksum & 0xFF00) >> 8;

    // Send the packet:
    dll_send_packet(node, packet_size);
}

void net_send_ping_response_packet(dll_address node) {
    const uint8_t packet_size = 5;
    // if (ping_request_packet_size > dll_get_data_buffer_size()) {
    if (packet_size > 128) {
        return;
    }

    // Get pointer to DLL data buffer:
    // uint8_t *packet = dll_get_data_buffer();
    uint8_t *packet = dll_create_data_buffer(0);

    // Write the packet's header:
    packet[PING_RESPONSE_PACKET_FIELD_CONTROL_L] = 0;
    packet[PING_RESPONSE_PACKET_FIELD_CONTROL_H] = (NET_PING_RESPONSE_PACKET << 4) | (NET_TX_CHECKSUM_TYPE << 2);
    packet[PING_RESPONSE_PACKET_FIELD_SOURCE_ADDRESS] = net_get_own_address();

    // Generate the checksum on the header:
    const uint8_t checksum_size = 3;
    uint16_t checksum = net_generate_checksum(NET_TX_CHECKSUM_TYPE, packet, checksum_size);

    // Write the checksum to the end of the packet:
    packet[packet_size - 2] = checksum & 0x00FF;
    packet[packet_size - 1] = (checksum & 0xFF00) >> 8;

    // Send the packet:
    dll_send_packet(node, packet_size);
}

static bool net_validate_packet(uint8_t *packet, uint8_t packet_length) {
    // Check that a packet has actually been received:
    if (packet == NULL || packet_length < 4) {
        return false;
    }

    // Extract packet type and checksum type from the control field:
    net_packet_type packet_type = (packet[PACKET_FIELD_CONTROL_H] & 0xF0) >> 4;
    net_checksum_type checksum_type = (packet[PACKET_FIELD_CONTROL_H] & 0x0C) >> 2;

    // Get the checksum from the last two bytes:
    uint16_t checksum = packet[packet_length - 2] | (packet[packet_length - 1] << 8);

    // Use the checksum to check and correct any errors in the packet:
    uint8_t checksumed_length = (packet_type == NET_DATA_PACKET) ? DATA_PACKET_FIELD_PAYLOAD_START : (packet_length - 2);
    bool checksum_passed = net_fix_checksum_errors(checksum_type, checksum, packet, checksumed_length);
    if (checksum_passed == false) {
        return false;
    }

    // Check that the packet's length is correct:
    switch (packet_type) {
        case NET_DATA_PACKET: {
            uint8_t payload_length = packet[DATA_PACKET_FIELD_PAYLOAD_LENGTH];
            uint8_t expected_packet_length = DATA_PACKET_FIELD_PAYLOAD_START + payload_length + 2;
            if (packet_length != expected_packet_length) {
                return false;
            }
        } break;

        case NET_LINK_STATE_PACKET: {
            uint8_t node_count = packet[LINK_STATE_PACKET_FIELD_NODE_COUNT];
            uint8_t expected_packet_length = LINK_STATE_PACKET_FIELD_NODE_LIST_START + node_count + 2;
            if (packet_length != expected_packet_length) {
                return false;
            }
        } break;

        case NET_PING_REQUEST_PACKET: {
            uint8_t expected_packet_length = 4;
            if (packet_length != expected_packet_length) {
                return false;
            }
        } break;

        case NET_PING_RESPONSE_PACKET: {
            uint8_t expected_packet_length = 5;
            if (packet_length != expected_packet_length) {
                return false;
            }
        } break;

        default: {
            return false;
        } break;
    }

    // If all the tests passed, the packet is valid:
    return true;
}

void net_handle_received_packet(dll_address previous_hop, uint8_t *packet, uint8_t packet_length) {
    // Make sure the packet is valid before continuing:
    bool is_packet_valid = net_validate_packet(packet, packet_length);
    if (is_packet_valid == false) {
        return;
    }

    // Extract the packet type from the CONTROL field:
    net_packet_type packet_type = (packet[PACKET_FIELD_CONTROL_H] & 0xF0) >> 4;

    switch (packet_type) {
        case NET_DATA_PACKET: {
            net_address destination = packet[DATA_PACKET_FIELD_DESTINATION_ADDRESS];
            if (destination != net_get_own_address()) {
                // If the packet isn't addressed to this node, send it on to the next hop.

                if (packet_length <= 128) {
                // if (packet_length <= dll_get_data_buffer_size()) {
                    dll_address next_hop = net_get_next_hop(destination);

                    // Check that the next hop was actually resolved, and only send the packet if it was:
                    if (next_hop != NET_NEXT_HOP_NOT_RESOLVED) {
                        // uint8_t *tx_packet = dll_get_data_buffer();
                        uint8_t *tx_packet = dll_create_data_buffer(0);
                        uint8_t *rx_packet = packet;
                        memmove(tx_packet, rx_packet, packet_length);
                        dll_send_packet(next_hop, packet_length);
                    }
                }
            } else {
                // If the packet is addressed to this node, pass it up the stack using the receive callback.
                if (receive_callback != NULL) {
                    net_address source = packet[DATA_PACKET_FIELD_SOURCE_ADDRESS];
                    uint8_t *payload = &packet[DATA_PACKET_FIELD_PAYLOAD_START];
                    uint8_t payload_length = packet[DATA_PACKET_FIELD_PAYLOAD_LENGTH];
                    receive_callback(source, payload, payload_length);
                }
            }
        } break;

        case NET_LINK_STATE_PACKET: {
            net_address source = packet[LINK_STATE_PACKET_FIELD_SOURCE_ADDRESS];
            uint8_t sequence_number = packet[LINK_STATE_PACKET_FIELD_SEQUENCE_NUMBER];
            uint8_t node_count = packet[LINK_STATE_PACKET_FIELD_NODE_COUNT];
            net_address *node_list = &packet[LINK_STATE_PACKET_FIELD_NODE_LIST_START];

            bool is_valid = net_notify_link_state_packet(source, sequence_number, node_list, node_count);

            if (is_valid == false) {
                // This packet has already been received before; don't continue flooding the packet.
                break;
            }

            // Broadcast the packet to all neighbouring nodes to continue flooding the packet:
            if (packet_length <= 128) {
            // if (packet_length <= dll_get_data_buffer_size()) {
                // uint8_t *tx_packet = dll_get_data_buffer();
                uint8_t *tx_packet = dll_create_data_buffer(0);
                uint8_t *rx_packet = packet;
                memmove(tx_packet, rx_packet, packet_length);

                // Send the packet to each connected neighbour (except the one that the packet came from):
                for (dll_address address = 0; address < DLL_BROADCAST_ADDRESS; address++) {
                    if (address != previous_hop && net_is_node_neighbour(address)) {
                        dll_send_packet(address, packet_length);
                    }
                }
            }
        } break;

        case NET_PING_REQUEST_PACKET: {
            // Send back a ping response:
            net_send_ping_response_packet(previous_hop);
        } break;

        case NET_PING_RESPONSE_PACKET: {
            // Pass on the information to the router:
            net_address logical_address = packet[PING_RESPONSE_PACKET_FIELD_SOURCE_ADDRESS];
            dll_address physical_address = previous_hop;
            net_notify_ping_response(physical_address, logical_address);
        } break;

        default: {
            // Ignore the packet.
            // Nothing to do.
        } break;
    }
}

void net_set_receive_callback(net_receive_callback callback) {
    receive_callback = callback;
}
