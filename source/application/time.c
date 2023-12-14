#include "time.h"
#include <avr/interrupt.h>
#include <util/atomic.h>

static volatile int32_t ms_counter;

void time_initialise() {
    cli();

    // Set up Timer 1 for 1000 Hz interrupts:
    // - Waveform mode = fast PWM with ICR1A as top
    // - Clock prescaler =  divide by 8
    // - ICR1 = 1499
    // - Enable overflow interrupt
    TCCR1A = (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
    TCCR1C = 0;
    ICR1 = 1499;
    TIMSK1 = (1 << TOIE1);
    TCNT1 = 0;

    ms_counter = 0;

    sei();
}

time time_now() {
    time millis_now;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        millis_now = ms_counter;
    }
    return millis_now;
}

time time_add_milliseconds(time t, int32_t milliseconds) {
    return t + milliseconds;
}

time time_add_seconds(time t, int32_t seconds) {
    return time_add_milliseconds(t, seconds * 1000);
}

time time_add_minutes(time t, int32_t minutes) {
    return time_add_milliseconds(t, minutes * 60000);
}

time time_add_hours(time t, int32_t hours) {
    return time_add_milliseconds(t, hours * 3600000);
}

int32_t time_delta_milliseconds(time start, time end) {
    return end - start;
}

int32_t time_delta_seconds(time start, time end) {
    return time_delta_milliseconds(start, end) / 1000;
}

int32_t time_delta_minutes(time start, time end) {
    return time_delta_milliseconds(start, end) / 60000;
}

int32_t time_delta_hours(time start, time end) {
    return time_delta_milliseconds(start, end) / 3600000;
}

ISR(TIMER1_OVF_vect) {
    ms_counter++;
}