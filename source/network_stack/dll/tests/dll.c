// Author: Oliver Enrico
// Team A1 DLL Implementation
// DLL/PHY Protocol v1.1 Implemented

#include "dll_private.h"
#include "uart.h"
#include "network_stack/phy.h"
#include <stdbool.h>

static uint8_t packet_buffer_tx[BUFSIZE] = {0};
static uint8_t packet_buffer_rx[BUFSIZE] = {0};
static uint8_t frame_buffer_tx[FRAMEBUFSIZE] = {0};
static uint8_t frame_buffer_rx[FRAMEBUFSIZE] = {0};

uint8_t received_packet_length = 0;
uint8_t sequence_number_counter;
uint8_t ack = 0;
uint8_t rtc = 0;
uint8_t ctc = 0;

dll_callback net_callback_ptr; // A pointer that will point to the net callback function

uint8_t *dll_create_data_buffer(uint8_t net_packet_length) {
    if(net_packet_length > 128) {
        //put_str("NO BUFFER ALLOCATED.\n");
        return NULL;
    }
    // The 128 byte packet buffer address is returned to NET
    //put_str("PACKET BUFFER ALLOCATED\n");
    return packet_buffer_tx;
}

// Sends data in the packet buffer to a given address (or broadcast for address 0xFF)
// Returns 1 if packet is successfully transmitted
// Returns 0 if a node is unreachable
// This function really only prepares frames for transmission
dll_send_response dll_send_packet(uint8_t destination_address, uint8_t packet_length) {

    if(packet_length > 128 || (destination_address == 0xFF && packet_length > 23)) {
        return DLL_PACKET_TOO_BIG;
    }

    uint8_t bytes_left_to_transmit = packet_length;
    bool multiple_frames = false;
    bool last_frame = true;
    uint8_t frame_number = 0;
    uint8_t sequence_number = 0;
    uint8_t frame_data_length;
    uint8_t checksum_result;
    
    if(packet_length > 23) {
        multiple_frames = true;
        last_frame = false;
        frame_data_length = 23;
    } else {
        frame_data_length = packet_length;
    }

    // Attempts to establish connection to designated node
    // Returns to NET layer with 0 if cannot establish connection
    if(establish_connection(destination_address)) {
        return DLL_NODE_UNREACHABLE;
    }

    //printf("\n");

    while(1) {
        put_str("\n\n----------------STARTING FRAME TRANSMISSION PROCESS---------------");
        /*put_str("\nFrame number: ");
        print_int(frame_number);

        put_str("\nFrame data length: ");
        print_int(frame_data_length);

        put_str("\nBytes left to transmit: ");;
        print_int(bytes_left_to_transmit);

        put_str("\nSequence Number: ");
        print_int(sequence_number);*/

        // Constructing frame

        //---PREPARING CONTROL FIELD---//

        frame_buffer_tx[FRAME_CONTROL_FIELD] = 0x00;
        frame_buffer_tx[FRAME_CONTROL_FIELD + 1] = 0b00000001;

        if(sequence_number) {
            // Sets bit 2 to 1 if the sequence number is 1
            frame_buffer_tx[FRAME_CONTROL_FIELD + 1] |= 1 << 2;
        }
    
        if(last_frame) {
            // Sets bit 3 to 1 if this is the last (end) frame
            frame_buffer_tx[FRAME_CONTROL_FIELD + 1] |= 1 << 3;
        }

        if(CHECKSUM_MODE == ERROR_EVEN_PARITY) {
            // Even bit parity
            frame_buffer_tx[FRAME_CONTROL_FIELD + 1] |= 1 << 4;
        }

        //---PREPARING ADDRESS FIELD---//
        
        frame_buffer_tx[FRAME_ADDRESS_FIELD] = destination_address;
        frame_buffer_tx[FRAME_ADDRESS_FIELD + 1] = NODE_HARDWARE_ADDRESS;

        //---PREPARING LENGTH AND DATA FIELDS---//

        if(!last_frame) {
            frame_buffer_tx[FRAME_LENGTH_FIELD] = 23;
            memcpy(&frame_buffer_tx[FRAME_DATA_FIELD], &packet_buffer_tx[frame_number * 23], 23);
        } else {
            frame_buffer_tx[FRAME_LENGTH_FIELD] = bytes_left_to_transmit;
            memcpy(&frame_buffer_tx[FRAME_DATA_FIELD], &packet_buffer_tx[frame_number * 23], bytes_left_to_transmit);
        }

        //---PREPARING CHECKSUM FIELD---//
    
        if(CHECKSUM_MODE == ERROR_EVEN_PARITY) {
            checksum_result = checksum_parity(&frame_buffer_tx[FRAME_CONTROL_FIELD], frame_buffer_tx[FRAME_LENGTH_FIELD] + 5, 0);
        }

        //put_str("\nChecksum result is: ");
        //print_int(checksum_result);

        frame_buffer_tx[FRAME_DATA_FIELD + frame_data_length] = (uint8_t) ((checksum_result >> 8) & 0xFF);
        frame_buffer_tx[FRAME_DATA_FIELD + frame_data_length + 1] = (uint8_t) (checksum_result & 0xFF);

        //put_str("\nData frame prepared and ready for byte stuffing.");
        /*put_str("Current frame buffer: ");
        print_buffer(&frame_buffer_tx[1], frame_data_length + 7);
        put_ch('\n');

        put_str("\nLength pre-stuffing is ");
        print_int(7 + frame_data_length);*/

        //---PERFORM BYTE STUFFING ON THE FRAME---//

        uint8_t finished_frame_length = byte_stuff_frame(7 + frame_data_length); // Length to be stuffed is the data, and 7 bytes of header (excluding the flag bytes)

        //put_str("\nByte stuffing completed.");
        /*put_str("New frame length is ");
        print_int(finished_frame_length);
        put_ch('\n');*/
        put_str("\nCurrent frame buffer: ");
        print_buffer(frame_buffer_tx, finished_frame_length);
        printf("\n");

        //---TRANSMIT THE FRAME---//

        /*if(mimic_transmit_frame(finished_frame_length)) {
            return DLL_NODE_UNREACHABLE;
        }*/

        if(transmit_frame(finished_frame_length)) {
            return DLL_NODE_UNREACHABLE;
        }
        put_str("\nWaiting for ACK...");
        while(1) {
            dll_check_for_transmission();
            if(ack) {
                ack = 0;
                break;
            }
        }
        
        // Once confirmation of frame reception is received
        if(multiple_frames && !last_frame) {
            sequence_number = !sequence_number;
            frame_number++;
            bytes_left_to_transmit -= 23;
            if(bytes_left_to_transmit < 24) {
                last_frame = true;
                frame_data_length = bytes_left_to_transmit;
            }
        } else {
            return DLL_TRANSMISSION_SUCCESS;
        }
    }
}

// Sets the NET layer function to be called when a frame is received
// Pass a pointer to the function that should be called
void dll_set_callback(dll_callback callback) {
    net_callback_ptr = callback;
}

// Performs byte stuffing on the frame
// Length of frame to stuff must be passed in
// Returns the resulting length of the frame
uint8_t byte_stuff_frame(uint8_t length) {
    frame_buffer_tx[FRAME_HEADER_FIELD] = FLAG_BYTE;
    frame_buffer_tx[length + 1] = FLAG_BYTE;

    uint8_t stuffed_length = length;

    int i;
    for(i=1; i<length; i++) {
        //printf("Stuffer testing byte at position %d, which has value 0x%X\n", i, frame_buffer_tx[i]);
        if((frame_buffer_tx[i] == FLAG_BYTE) || (frame_buffer_tx[i] == ESCAPE_BYTE)) {
            // If this is entered, the current byte needs escaping
            //put_str("\nBAD BYTE DETECTED, MUST BE STUFFED");
            int x;
            for(x=stuffed_length + 1; x>=i; x--) { // Starts at 2 above stuffed_length, as need to insert the FLAG byte, which is already one index above the max
                frame_buffer_tx[x + 1] = frame_buffer_tx[x]; // Shift all elements from problem byte forwards
            }
            frame_buffer_tx[i] = ESCAPE_BYTE; // Insert escape byte
            stuffed_length++;
            i++;
        }
    }
    return stuffed_length + 2;
}

uint8_t establish_connection(uint8_t address) {
    return 0;
    sequence_number_counter = 0;
    uint8_t control_frame_stuffed_length = prepare_control_frame(address, CONTROL_RTC);
    put_str("\nTransmitting control frame: RTC");
    transmit_frame(control_frame_stuffed_length);

    uint8_t length;

    put_str("\nWaiting for CTC...");
    while(1) { 
        dll_check_for_transmission();
        if(ctc) {
            ctc = 0;
            break;
        }
    }
    return 0;
}

// Prepares frame_buffer_tx with a control frame
// Pass the destination address, and a control frame type to the function
// Does byte stuffing on frame
// Returns length of frame
uint8_t prepare_control_frame(uint8_t address, control_frame_types type) {
    // Setting first control byte
    frame_buffer_tx[FRAME_CONTROL_FIELD] = type;
    frame_buffer_tx[FRAME_CONTROL_FIELD + 1] = 0;
    if(type == CONTROL_ACK) {
        frame_buffer_tx[FRAME_CONTROL_FIELD + 1] |= sequence_number_counter << 2;
    }
    if(CHECKSUM_MODE == ERROR_EVEN_PARITY) {
        frame_buffer_tx[FRAME_CONTROL_FIELD + 1] |= 1 << 4;
    }
    frame_buffer_tx[FRAME_ADDRESS_FIELD] = address;
    frame_buffer_tx[FRAME_ADDRESS_FIELD + 1] = NODE_HARDWARE_ADDRESS;
    //Control frames do not have length of data fields
    //So checksum is placed in length field and first byte of data field
    uint16_t checksum_result;
    if(CHECKSUM_MODE == ERROR_EVEN_PARITY) {
        checksum_result = checksum_parity(&frame_buffer_tx[FRAME_CONTROL_FIELD], 4, 0);
    }
    frame_buffer_tx[FRAME_LENGTH_FIELD] = (uint8_t) ((checksum_result >> 8) & 0xFF);
    frame_buffer_tx[FRAME_DATA_FIELD] = (uint8_t) (checksum_result & 0xFF);
    return byte_stuff_frame(6); // Unstuffed length of all control frames is 6 bytes
}

// Returns 0 if the frame is successfully transmitted
uint8_t transmit_frame(uint8_t length) {
    put_str("\nTransmitting frame...");
    print_buffer(frame_buffer_tx, length);
    phy_transmit_frame(frame_buffer_tx, length);
    return 0;
}

// Returns 0 if the frame is successfully transmitted
uint8_t mimic_transmit_frame(uint8_t length) {
    //Currently mimics transmission to the same node
    put_str("\nMimicking frame transmission...\n");
    memcpy(frame_buffer_rx, frame_buffer_tx, length);

    uint8_t i;
    for(i=0; i<FRAMEBUFSIZE; i++) {
        frame_buffer_tx[i] = 0; // Clears transmission buffer
    }

    receive_frame(length);
    return 0;
}

void dll_check_for_transmission() {
    //put_str("\nChecking...");
    uint8_t length = phy_receive_frame(frame_buffer_rx, sizeof(frame_buffer_rx));
    //put_str("\nLength is ");
    //print_int(length);
    if(length) {
        //put_str("\nFrame received! Length is ");
        //print_int(length);
        //put_ch('\n');
        receive_frame(length);
    }
}

void dll_update() {
    dll_check_for_transmission();
}

// To be called once data has been received and placed into frame_buffer_rx
void receive_frame(uint8_t length) {
    put_str("\n\n----------------STARTING FRAME RECEIVING PROCESS---------------");
    put_str("\nReceived buffer size: ");
    print_int(length);
    put_str("\nReceived frame buffer: ");
    print_buffer(frame_buffer_rx, length);

    uint8_t frame_length = byte_unstuff_frame(length);
    //print_int(frame_length);
    received_packet_length += frame_length - 7;

    frame_receive_process_responses process_result = process_received_frame(frame_length);

    if(process_result == ADDRESS_MISMATCH) {
        //put_str("\nFrame not addressed to this node.");
        return;
    } else if(process_result == FINAL_FRAME) {
        memcpy(&packet_buffer_rx[received_packet_length - frame_length + 7], &frame_buffer_rx[FRAME_DATA_FIELD], frame_length - 7);
        //put_str("\nThis is the final frame");
        (*net_callback_ptr)(frame_buffer_rx[FRAME_ADDRESS_FIELD + 1], packet_buffer_rx, received_packet_length);
        received_packet_length = 0;
        sequence_number_counter = 0;
    } else if(process_result == MORE_FRAMES_EXPECTED) {
        memcpy(&packet_buffer_rx[received_packet_length - frame_length + 7], &frame_buffer_rx[FRAME_DATA_FIELD], frame_length - 7);
        put_str("\nMore frames expected.");
    } else if(process_result == CONTROL_FRAME) {
        control_frame_types type = frame_buffer_rx[FRAME_CONTROL_FIELD];
        if(type == CONTROL_ACK) {
            put_str("\nAck received.");
            ack = 1;
        } else if(type == CONTROL_RTC) {
            rtc = 1;
        } else if(type == CONTROL_CTC) {
            ctc = 1;
        }
    }
    //put_ch('\n');
}

// Performs byte unstuffing on the received frame
// Length of frame to unstuff should be passed in. Strictly it is not necessary, but it is compared to what DLL thinks the frame length is
// Returns the resulting length of the frame
uint8_t byte_unstuff_frame(uint8_t length) {
    //put_str("\n\nUnstuffing received frame...");
    uint8_t unstuffed_length = length - 2;
    //Remove first flag byte
    if(frame_buffer_rx[FRAME_HEADER_FIELD] == FLAG_BYTE) {
        frame_buffer_rx[FRAME_HEADER_FIELD] = 0;
    } else {
        //put_str("\nError here");
        return 0; //Error
    }

    //Find second occurence of flag byte that is not escaped
    //Must check if flag byte is preceded by escape byte, and if the escape byte itself is escaped
    //Also removes any escape bytes preceding a flag or escape byte
    uint8_t measured_length = 1;
    uint8_t i;
    for(i=1; i<FRAMEBUFSIZE - 1; i++) {
        if((frame_buffer_rx[i] == ESCAPE_BYTE) && ((frame_buffer_rx[i+1] == ESCAPE_BYTE) || (frame_buffer_rx[i+1] == FLAG_BYTE))) {
            put_str("\nRemoving escape byte...");
            int x;
            for(x=i; x<FRAMEBUFSIZE; x++) {
                frame_buffer_rx[x] = frame_buffer_rx[x+1]; // Shift all elements from problem byte backwards
                //memory
            }
            unstuffed_length--;
            measured_length++;
        }
        measured_length++;

        if((frame_buffer_rx[i] == FLAG_BYTE) && (frame_buffer_rx[i-1] != ESCAPE_BYTE)) {
            //put_str("\nSecond flag byte found at position ");
            //print_int(i);
            break;
        }
    }
    
    //put_str("\nMeasured frame length is: ");
    //print_int(measured_length);

    if(measured_length != length) {
        //put_str("\nMeasured frame length is not equal to length passed to function!");
        return 0;
    } else {
        //put_str("\nMeasured frame length matches length passed to function!");
    }
    //put_str("\nLength of unstuffed frame: ");
    return unstuffed_length;
}

//Confirms node is intended recipient, validates data with checksum, checks type of frame, and sends acknowledgment if all is okay
//Needs length of frame to be passed in
frame_receive_process_responses process_received_frame(uint8_t length) {

    //Check node is intended recipient
    if(frame_buffer_rx[FRAME_ADDRESS_FIELD] != NODE_HARDWARE_ADDRESS) {
        //Frame is intended for other node, so is discarded
        put_str("\nFrame is addressed to a different node, discarding.");
        uint8_t i;
        for(i=0; i<FRAMEBUFSIZE; i++) {
            //Frame buffer is reset
            frame_buffer_rx[i] = 0;
        }
        return ADDRESS_MISMATCH;
    }

    //put_str("\nFrame is addressed to this node!");

    //Run checksum on frame
    //put_str("\nRunning checksum on frame...");
    //First identify the type of checksum indicated in the error control bits
    uint8_t error_type = 0;
    uint8_t i;
    for(i=7; i>3; i--) {
        error_type = (error_type << 1) | (frame_buffer_rx[FRAME_CONTROL_FIELD + 1] & 1 << i) >> 4;
    }
    //put_str("\nError type field is set to ");
    //print_int(error_type);

    uint8_t frame_type = frame_buffer_rx[FRAME_CONTROL_FIELD + 1] & 1 << 0; // 1 is data frame, 0 is control frame
    //put_str("\nFrame type is ");
    //print_int(frame_type);

    uint8_t checksum_result;
    if(error_type == ERROR_EVEN_PARITY) {
        if(frame_type) {
            //put_str("\nEven parity identified as error checking method");
            checksum_result = checksum_parity(&frame_buffer_rx[FRAME_CONTROL_FIELD], frame_buffer_rx[FRAME_LENGTH_FIELD] + 5, 0);
            //put_str("\nChecksum result is ");
            //print_int(checksum_result);
            if(checksum_result == frame_buffer_rx[FRAME_DATA_FIELD + frame_buffer_rx[FRAME_LENGTH_FIELD] + 1]) {
                //put_str("\nChecksum match!");
            } else {
                //put_str("\nChecksum is different!");
            }
        } else {
            checksum_result = checksum_parity(&frame_buffer_rx[FRAME_CONTROL_FIELD], 4, 0);
            //put_str("\nChecksum result is ");
            //print_int(checksum_result);
            if(checksum_result == frame_buffer_rx[FRAME_DATA_FIELD]) {
                
                //put_str("\nChecksum match!");
            } else {
                //put_str("\nChecksum is different!");
            }
        }
    }

    if(!frame_type) { //Tests for type=0
        //Control frame
        put_str("\nFrame is a control frame.");
        return CONTROL_FRAME;
    }

    //If we get here, the frame is a data frame
    put_str("\nFrame is a data frame.");

    uint8_t frame_sequence_number = (frame_buffer_rx[FRAME_CONTROL_FIELD + 1] & 1 << 2) >> 2;
    //put_str("\nFrame sequence number is ");
    //print_int(frame_sequence_number);

    if(sequence_number_counter != frame_sequence_number) {
        //put_str("\nSequence number is not as expected!");
    } else {
        //put_str("\nSequence number correct!");
    }

    uint8_t size = prepare_control_frame(frame_buffer_rx[FRAME_ADDRESS_FIELD + 1], CONTROL_ACK);
    transmit_frame(size);

    sequence_number_counter = !sequence_number_counter;

    uint8_t final_bit = (frame_buffer_rx[FRAME_CONTROL_FIELD + 1] & 1 << 3) >> 3; //Tests if End=1

    if(final_bit) {
        return FINAL_FRAME;
    } else {
        return MORE_FRAMES_EXPECTED;
    }
} 

// Returns a 16-bit integer represnting the 2 bytes of the checksum field
// Pass a pointer to the start of the packet data and the length in bytes to checksum over
uint16_t checksum_CRC16_CCITT(uint8_t *ptr, uint8_t length) {
    return 0;
}

// Returns a 16-bit integer represnting the 2 bytes of the checksum field
// Pass a pointer to the start of the packet data, the length in bytes to checksum over, and a boolean (even=0, odd=1)
uint16_t checksum_parity(uint8_t *ptr, uint8_t length, uint8_t type) {
    uint16_t parity = 0;
    int i;
    for(i=0; i<length; i++) { //Loops through each byte passed in
        if(byte_parity(ptr[i])) {
            parity = !parity; //If a byte returns 1 (odd number of ones), parity is inverted
        }
    }

    if(type) {
        return !parity; //Type allows for odd or even parity
    }
    return parity;
}

// Returns 0 or 1 indicating whether an even parity bit is needed (1 is returned for odd numbers of 1s)
int byte_parity(uint8_t byte) {
    uint8_t bit;
    int parity = 0;
    int i;
    for(i=0; i<8; i++) {
        bit = 1 << i; //Creates a bitmask
        if(bit & byte) {
            parity = !parity; //Parity is inverted if bitwise and gives true (inverts for every 1 in byte)
        }
    }
    return parity; //As parity started at zero, if it inverts an even number of times, it will return 0
}