#include "time.h"
#include <avr/io.h>

int main() {
    time_initialise();

    // Set LED pin as output:
    DDRB |= (1 << PB7);

    // Initialise next_time:
    time next_time = time_now();

    while (1) {

        // Wait until next time:
        while (time_delta_milliseconds(next_time, time_now()) < 0);

        // Increment next time by 500 miliseconds:
        next_time = time_add_milliseconds(next_time, 500);

        // Toggle LED pin:
        PINB |= (1 << PB7);

    }
}
