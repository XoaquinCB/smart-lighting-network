#include "network_stack/phy.h"
#include "uart.h"
#include <stdint.h>
#include <util/delay.h>

uint8_t rx_buffer[PHY_MAX_RX_FRAME_SIZE];
const uint8_t tx_data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };

int main() {
    phy_initialise();
    uart_initialise();

    uart_put_string("\n\rStarted.\n\r");

    uint16_t counter = 0;
    while (1) {
        uint8_t rx_length = phy_receive_frame(rx_buffer, sizeof(rx_buffer));
        if (rx_length != 0) {
            uart_put_string("\n\rPacket received\n\r  Length: 0x");
            uart_print_hex_8(rx_length);
            uart_put_string("\n\r  Data: ");
            for (uint8_t i = 0; i < rx_length; i++) {
                uart_print_hex_8(rx_buffer[i]);
                uart_put_byte(' ');
            }
            uart_put_string("\n\r");
        }
        _delay_ms(1);

        if (counter++ == 2000) {
            counter = 0;

            uart_put_string("\n\rStarting transmission...");
            bool transmitted = phy_transmit_frame(tx_data, sizeof(tx_data));
            if (transmitted) {
                uart_put_string("\n\rPacket transmitted.\n\r  Length: 0x");
                uart_print_hex_8(sizeof(tx_data));
                uart_put_string("\n\r  Data: ");
                for (uint8_t i = 0; i < sizeof(tx_data); i++) {
                    uart_print_hex_8(tx_data[i]);
                    uart_put_byte(' ');
                }
                uart_put_string("\n\r");
            } else {
                uart_put_string("\n\rPacket transmission failed.\n\r");
            }
        }
    }
}