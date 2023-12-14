#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * The maximum number of bytes that can be received in one frame.
 */
#define PHY_MAX_RX_FRAME_SIZE 59

/**
 * @brief Initialises the physical layer.
 */
void phy_initialise();

/**
 * @brief Transmits a frame of data to all connected nodes.
 * @param data: The frame of data to transmit.
 * @param length: The number of bytes to transmit.
 * @returns 'true' if the frame was transmitted; 'false' if the channel is busy.
 */
bool phy_transmit_frame(const uint8_t *data, uint8_t length);

/**
 * @brief Returns a frame of data if one has been received.
 * @param output_buffer: A pointer to a buffer where the frame will be copied to.
 * @param max_length: The maximum number of bytes to copy to the output buffer.
 * @returns The number of bytes copied to the output buffer. This will not be larger than
 *         'max_length' or PHY_MAX_RX_FRAME_SIZE'. If no frame has been received, zero is returned.
 */
uint8_t phy_receive_frame(uint8_t *output_buffer, uint8_t max_length);
