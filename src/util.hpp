#pragma once
#include <avr/io.h>
#include <avr/wdt.h>

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

/**
 * Disable the watchdog timer and clear the watchdog timer reset flag.
 *
 * @note
 * required, as per datasheet (WDE bit explanation on P. 49):
 * WDE is overridden by WDRF in MCUSR.
 * This means that WDE is always set when WDRF is set.
 * To clear WDE, WDRF must be cleared first.
 */
inline void wdt_disable_actual()
{
    MCUSR &= ~_BV(WDRF); // clear WDT reset flag
    wdt_disable();       // sets WDE to 0
}
