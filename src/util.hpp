#pragma once
#include <avr/io.h>

#define IS_ATTINY13 (defined(__AVR_ATtiny13__) || defined (__AVR_ATtiny13A__))

/**
 * Wait for the specified number of seconds.
 *
 * @param seconds the number of seconds to wait for
 *
 * @note
 * this function doesn't disable any peripherals. if desired, disable them before calling this function.
 *
 * @note
 * this function alters the watchdog timer configuration.
 * additionally, after this function returns, the watchdog timer is disabled.
 *
 * @note
 * after this function returns, interrupts are disabled.
 *
 * @note
 * loosely based on https://electronics.stackexchange.com/a/151743
 */
void sleep_for(const uint32_t seconds);

/**
 * Reset the CPU.
 */
void reset_cpu();