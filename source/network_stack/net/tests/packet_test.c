#include "network_stack/net.h"
#include "network_stack/dll.h"
#include "uart.h"
#include <stddef.h>
#include <util/delay.h>

int main() {
    uart_initialise();
    net_initialise();
    phy_initialise();

    uart_put_string("\n\rOwn NET address: ");
    uart_print_hex_8(net_get_own_address());
    uart_put_string("\n\r");

    uint16_t counter = 0;
    while (1) {
        net_update();
        dll_update();

        _delay_ms(1);
        if (counter++ == 10000) {
            counter = 0;

            const uint8_t buffer_size = 40;
            uint8_t *buffer = net_create_data_buffer(buffer_size);
            if (buffer != NULL) {
                uart_put_string("Allocated NET buffer.\n\r");

                for (uint8_t i = 0; i < buffer_size; i++) {
                    buffer[i] = i;
                }

                net_address destination = 0x72;
                uart_put_string("Transmitting NET packet...\n\r");
                net_send_data_buffer(buffer, destination);
                uart_put_string("Transmitted NET packet.\n\r");
            } else {
                uart_put_string("Failed to allocated NET buffer.\n\r");
            }
        }
    }
}
