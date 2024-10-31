#include "util.hpp"
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "print.hpp"

#if IS_ATTINY13
    EMPTY_INTERRUPT(WDT_vect);

    void sleep_for(const uint32_t seconds)
    {
        PRINT_PREFIX();
        PRINT(F("sleep_for() seconds="));
        PRINTLN(seconds);

        cli();                // disable interrupts
        wdt_disable_actual(); // clear wdt reset flag and ensure WDT is known configuration

        // sleep for the specified number of seconds
        for (uint32_t i = 0; i < seconds; i++)
        {
            // setup WDT
            WDTCR |= _BV(WDCE) | _BV(WDE); // allow changes, enable WDT
            WDTCR = _BV(WDTIE) | WDTO_1S;  // set WDT to interrupt only mode with 1s timeout

            // sleep until WDT interrupt
            set_sleep_mode(SLEEP_MODE_PWR_DOWN); // deepest sleep mode, only WDT can wake up the MCU
            wdt_reset();                         // start wdt at zero
            sei();                               // enable interrupts
            sleep_mode();                        // sleep until WDT interrupt
        }

        // disable interrupts and WDT
        cli();
        wdt_reset();
        wdt_disable_actual();
    }
#else
    // use a stub for testing
    #include "util/delay.h"

    void sleep_for(const uint32_t seconds)
    {
        PRINT_PREFIX();
        PRINT(F("sleep_for() seconds="));
        PRINTLN(seconds);

        for(uint32_t i = 0; i < seconds; i++)
        {
            _delay_ms(1000);
        }
    }
#endif

void reset_cpu()
{
    PRINT_PREFIX();
    PRINTLN(F("reset_cpu()"));

    // to reset the cpu, we enable the watchdog timer with a very short timeout.
    // then, we hang in a loop until the watchdog timer resets the CPU for us.

    // note: wdt_enable resets WDTIE, so we don't need to disable it explicitly
    wdt_disable_actual();
    wdt_enable(WDTO_15MS);
    while (1)
        ;
}
