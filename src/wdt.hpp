/**
 * WDT helper module, provides functions for sleep-waiting and cpu software reset
 */
#pragma once
#include <stdint.h> // uint32_t

namespace wdt
{
    typedef bool (*sleep_tick_callback_t)(const uint32_t seconds_elapsed, const uint32_t sleep_time);

    /**
     * Wait for the specified number of seconds, spending as much time as possible in sleep mode.
     *
     * @param seconds the number of seconds to wait for
     * @param tick_callback optional callback function called after each second of sleep, allowing early sleep exit.
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
     * and https://electronics.stackexchange.com/a/74850
     */
    void sleep_for(uint32_t seconds, sleep_tick_callback_t tick_callback = nullptr);

    /**
     * signal the sleep_for routine that the last wakeup was not caused by
     * it's configured WDT interrupt, but by another interrupt.
     */
    void wakeup_was_not_wdt();

    /**
     * Disable the watchdog timer and clear the watchdog timer reset flag.
     *
     * @note
     * required, as per datasheet (WDE bit explanation on P. 49):
     * WDE is overridden by WDRF in MCUSR.
     * This means that WDE is always set when WDRF is set.
     * To clear WDE, WDRF must be cleared first.
     */
    void disable_actual();

    /**
     * Reset the CPU.
     */
    void reset_cpu();
} // namespace wdt
