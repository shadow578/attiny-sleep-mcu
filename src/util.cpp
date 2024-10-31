#include "util.hpp"
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "print.hpp"

#if IS_ATTINY13
    EMPTY_INTERRUPT(WDT_vect);

    void sleep_for(const uint32_t seconds)
    {
        PRINT_PREFIX();
        PRINT(F("sleep_for() seconds="));
        PRINTLN(seconds);

        cli();

        // enable the watchdog timer with a timeout of 8s, in interrupt mode
        wdt_reset();
        MCUSR &= ~_BV(WDRF);                     // clear wdt reset flag
        WDTCR = _BV(WDE) | _BV(WDCE);            // enable WDT
        WDTCR = _BV(WDE) | _BV(WDTIE) | WDTO_8S; // enable WDT interrupt, set timeout to 8s

        // sleep for the specified time
        for (uint32_t i = 0; i < seconds; i += 8)
        {
            cli();                               // disable interrupts
            set_sleep_mode(SLEEP_MODE_PWR_DOWN); // only WDT can wake up the MCU
            wdt_reset();                         // start wdt at zero
            sei();                               // enable interrupts
            sleep_mode();                        // sleep until WDT interrupt
            sleep_disable();
        }

        cli();

        // disable WDT
        wdt_reset();
        wdt_disable();
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
    wdt_enable(WDTO_15MS);
    while (1)
        ;
}
