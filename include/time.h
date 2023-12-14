#pragma once

#include <stdint.h>

/**
 * Represents a specific point in time, as an arbitrary unit.
 */
typedef int32_t time;

/**
 * The time at which the library was initialised.
 */
#define TIME_ZERO ((time)0)

/**
 * @brief Initialises the time library.
 */
void time_initialise();

/**
 * @brief Returns the current time, relative to when the library was initialised.
 * @returns The current time.
 */
time time_now();

/**
 * @brief Adds a given number of milliseconds to a time.
 * @param t: The time to add to.
 * @param milliseconds: The number of milliseconds to add.
 * @returns The new time.
 */
time time_add_milliseconds(time t, int32_t milliseconds);

/**
 * @brief Adds a given number of seconds to a time.
 * @param t: The time to add to.
 * @param seconds: The number of seconds to add.
 * @returns The new time.
 */
time time_add_seconds(time t, int32_t seconds);

/**
 * @brief Adds a given number of minutes to a time.
 * @param t: The time to add to.
 * @param minutes: The number of minutes to add.
 * @returns The new time.
 */
time time_add_minutes(time t, int32_t minutes);

/**
 * @brief Adds a given number of hours to a time.
 * @param t: The time to add to.
 * @param hours: The number of hours to add.
 * @returns The new time.
 */
time time_add_hours(time t, int32_t hours);

/**
 * @brief Calculates the number of milliseconds between two times.
 * @param start: The starting time.
 * @param end: The ending time.
 * @returns The number of milliseconds from the start time to the end time.
 */
int32_t time_delta_milliseconds(time start, time end);

/**
 * @brief Calculates the number of seconds between two times.
 * @param start: The starting time.
 * @param end: The ending time.
 * @returns The number of seconds from the start time to the end time.
 */
int32_t time_delta_seconds(time start, time end);

/**
 * @brief Calculates the number of minutes between two times.
 * @param start: The starting time.
 * @param end: The ending time.
 * @returns The number of minutes from the start time to the end time.
 */
int32_t time_delta_minutes(time start, time end);

/**
 * @brief Calculates the number of hours between two times.
 * @param start: The starting time.
 * @param end: The ending time.
 * @returns The number of hours from the start time to the end time.
 */
int32_t time_delta_hours(time start, time end);
