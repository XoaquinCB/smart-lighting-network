#include "network_stack/phy.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "tests/uart.h"

static const uint8_t *tx_next_byte;
static volatile uint8_t tx_remaining;
static volatile bool tx_complete;

static uint8_t rx_buffer[PHY_MAX_RX_FRAME_SIZE];
static volatile uint8_t rx_length;
static volatile bool rx_complete;

void phy_initialise() {
    cli();

    // Set SCL frequency to 100 kHz:
    TWBR = 13;
    TWSR = (1 << TWPS0);

    // Enable recognition of general-call address:
    TWAR = (1 << TWGCE);

    // Enable I2C and acknowledge bit and interrupt:
    TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWIE);

    // Enable pullup resistors:
    PORTC |= (1 << PC1) | (1 << PC0);

    sei();
}

bool phy_transmit_frame(const uint8_t *data, uint8_t length) {
    tx_next_byte = data;
    tx_remaining = length;
    tx_complete = false;

    // Start a transmission:
    TWCR |= (1 << TWINT) | (1 << TWSTA);

    // Transmission will be handled by interrupt routine...

    // Wait for transmission to complete:
    while (!tx_complete);

    // If 'tx_remaining' isn't zero, the frame wasn't fully transmitted so it must have failed:
    bool success = (tx_remaining == 0);
    return success;
}

uint8_t phy_receive_frame(uint8_t *output_buffer, uint8_t max_length) {
    // Disable TWI interrupt:
    TWCR &= ~(1 << TWIE);

    uint8_t copied_length;

    if (rx_complete) {
        rx_complete = false;
        uint8_t count = (rx_length < max_length) ? rx_length : max_length;
        if (output_buffer != NULL) {
            memcpy(output_buffer, rx_buffer, count);
        }
        return count;
    } else {
        copied_length = 0;
    }

    // Re-enable TWI interrupt:
    TWCR |= (1 << TWIE);

    return copied_length;
}

ISR(TWI_vect) {
    uint8_t twi_status = TWSR & 0xF8;

    switch (twi_status) {

        // START or repeated-START condition has been transmitted:
        case 0x08:
        case 0x10: {
            // Transmit general-call address + W:
            TWDR = 0;

            // Clear the start bit:
            TWCR &= ~(1 << TWSTA);

            // Clear interrupt flag:
            TWCR |= (1 << TWINT);
        } break;

        // SLA+W transmitted, or data transmitted, ACK or NACK received:
        case 0x18:
        case 0x20:
        case 0x28:
        case 0x30: {
            if (tx_remaining != 0) {
                // Set up next byte to transmit:
                TWDR = *tx_next_byte;
                tx_next_byte++;
                tx_remaining--;

                // Clear interrupt flag:
                TWCR |= (1 << TWINT);
            } else {
                // Set the STOP bit and clear the interrupt flag:
                TWCR |= (1 << TWSTO) | (1 << TWINT);

                // Signal transmit complete:
                tx_complete = true;
            }
        } break;

        // Arbitration lost:
        case 0x38: {
            // Signal transmit complete (but failed):
            tx_complete = true;

            // Clear interrupt flag:
            TWCR |= (1 << TWINT);
        } break;

        // Arbitration lost, but then this device is addressed:
        case 0x68:
        case 0x78: {
            // Signal transmit complete (but failed):
            tx_complete = true;
        } // fallthrough...

        // Own SLA+W received, or general call address received:
        case 0x60:
        case 0x70: {
            // Reset RX length:
            rx_length = 0;

            // Clear interrupt flag:
            TWCR |= (1 << TWINT);
        } break;

        // Data byte received:
        case 0x80:
        case 0x90:
        case 0x88:
        case 0x98: {
            uint8_t data_byte = TWDR;

            if (rx_length < sizeof(rx_buffer)) {
                rx_buffer[rx_length] = data_byte;
                rx_length++;
            }

            // Clear interrupt flag:
            TWCR |= (1 << TWINT);
        } break;

        // STOP condition received:
        case 0xA0: {
            // Signal RX completed:
            rx_complete = true;

            // Clear interrupt flag:
            TWCR |= (1 << TWINT);
        } break;

        default: {
            // Clear interrupt flag:
            TWCR |= (1 << TWINT);
        } break;
    }
}