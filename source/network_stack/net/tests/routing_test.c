#include "network_stack/net.h"
#include "network_stack/dll.h"
#include "time.h"
#include "uart.h"
#include "../routing.h"
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * This test emulates the following network graph:
 *
 *          02 .  . 03
 *        .(12)    (13).
 *      .                .
 *    01 .  .  .  .  .  . 04
 *   (11)                (14)
 *   own  .            .
 *  address .        .
 *            .    .
 *              05
 *             (15)
 *
 * The node numbers outside the brackets correspond to the nodes' logical (net) addresses in hex. The node numbers in
 * brackets correspond to the nodes' physical (dll) addresses in hex. This device is the node on the very left with
 * logical address 0x01.
 */

time current_time = TIME_ZERO;

bool ping_request_0x12 = false;
bool ping_request_0x14 = false;
bool ping_request_0x15 = false;

uint8_t sequence_number_0x01 = 0;
uint8_t sequence_number_0x02 = 0;
uint8_t sequence_number_0x03 = 0;
uint8_t sequence_number_0x04 = 0;
uint8_t sequence_number_0x05 = 0;

const uint8_t node_list_0x02[] = { 0x03, 0x01 };
const uint8_t node_list_0x03[] = { 0x02, 0x04 };
const uint8_t node_list_0x04[] = { 0x03, 0x01, 0x05 };
const uint8_t node_list_0x05[] = { 0x01, 0x04 };

int main() {
    uart_initialise();
    uart_put_string("\n\r============================================================\n\r");
    uint8_t second_counter_100 = 0;
    uint8_t second_counter_10 = 0;

    net_initialise_routing();

    while (1) {
        _delay_ms(1000);
        current_time = time_add_seconds(current_time, 1);
        net_update_routing();

        // Emulate ping responses after certain amount of time:
        if (ping_request_0x12 && second_counter_100 > 25) {
            ping_request_0x12 = false;
            net_notify_ping_response(0x12, 0x02);
            uart_put_string("Emulating ping response from 0x02\n\r");
        }
        if (ping_request_0x14 && second_counter_100 > 65) {
            ping_request_0x14 = false;
            net_notify_ping_response(0x14, 0x04);
            uart_put_string("Emulating ping response from 0x04\n\r");
        }
        if (ping_request_0x15 && second_counter_100 > 85) {
            ping_request_0x15 = false;
            net_notify_ping_response(0x15, 0x05);
            uart_put_string("Emulating ping response from 0x05\n\r");
        }

        if (second_counter_100++ >= 100) {
            second_counter_100 = 0;
        }

        // Every 10 seconds, offset 0 seconds:
        if (second_counter_10 == 0) {
            // Print out the next hop of every destination:
            uart_put_string("Next hops:\n\r  Node Online Next hop \n\r");
            for (net_address node = 0; node <= NET_MAX_ADDRESS; node++) {
                uart_put_string("  ");
                uart_print_hex_8(node);
                uart_put_string("   ");
                uart_print_hex_8(net_is_device_online(node));
                uart_put_string("     ");
                uart_print_hex_8(net_get_next_hop(node));
                uart_put_string("\n\r");
            }
        }

        // Every 10 seconds, offset 3 seconds:
        if (second_counter_10 == 3) {
            // Emulate receiving a link state packet from node 0x02:
            net_notify_link_state_packet(0x02, sequence_number_0x02++, node_list_0x02, sizeof(node_list_0x02));
            uart_put_string("Emulating link state packet from 0x02\n\r");
        }

        // Every 10 seconds, offset 5 seconds:
        if (second_counter_10 == 5) {
            // Emulate receiving a link state packet from node 0x05:
            net_notify_link_state_packet(0x05, sequence_number_0x05++, node_list_0x05, sizeof(node_list_0x05));
            uart_put_string("Emulating link state packet from 0x05\n\r");

            // Emulate receiving a link state packet from node 0x04:
            net_notify_link_state_packet(0x04, sequence_number_0x04++, node_list_0x04, sizeof(node_list_0x04));
            uart_put_string("Emulating link state packet from 0x04\n\r");
        }

        // Every 10 seconds, offset 3 seconds:
        if (second_counter_10 == 8) {
            // Emulate receiving a link state packet from node 0x03:
            net_notify_link_state_packet(0x03, sequence_number_0x03++, node_list_0x03, sizeof(node_list_0x03));
            uart_put_string("Emulating link state packet from 0x03\n\r");
        }

        if (second_counter_10++ >= 10) {
            second_counter_10 = 0;
        }
    }
}

//*************************** net.h emulated implementation *************************//

net_address net_get_own_address() {
    // Use 0x01 as our own address:
    return 0x01;
}

//************************** time.h emulated implementation *************************//

time time_now() {
    return current_time;
}

int32_t time_delta_seconds(time start, time end) {
    return (end - start) / 1000;
}

time time_add_seconds(time t, int32_t seconds) {
    return t + seconds * 1000;
}

//************************* packets.h emulated implementation ***********************//

void net_send_ping_request_packet(dll_address node) {
    uart_put_string("Send ping request packet to physical address ");
    uart_print_hex_8(node);
    uart_put_string("\n\r");

    // Set flags to emulate responses from the neighbouring nodes:
    if (node == 0x12 || node == DLL_BROADCAST_ADDRESS) {
        ping_request_0x12 = true;
    }
    if (node == 0x14 || node == DLL_BROADCAST_ADDRESS) {
        ping_request_0x14 = true;
    }
    if (node == 0x15 || node == DLL_BROADCAST_ADDRESS) {
        ping_request_0x15 = true;
    }
}

void net_send_link_state_packet() {
    uart_put_string("Send link state packet:\n\r  Linked nodes: ");

    net_address own_address = net_get_own_address();
    for (net_address node = 0x00; node != NET_MAX_ADDRESS + 1; node++) {
        // If the node is linked, add it to the list:
        bool is_linked = net_are_nodes_linked(own_address, node);
        if (is_linked) {
            uart_print_hex_8(node);
            uart_put_string(", ");
        }
    }
    uart_put_string("\n\r");
}
