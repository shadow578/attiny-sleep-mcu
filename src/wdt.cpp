#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include "wdt.hpp"

EMPTY_INTERRUPT(WDT_vect);

static volatile bool ignore_wakeup = false;

void wdt::disable_actual()
{
    MCUSR &= ~_BV(WDRF); // clear WDT reset flag
    wdt_disable();       // sets WDE to 0
}

void wdt::sleep_for(uint32_t seconds)
{
    cli();                // disable interrupts
    disable_actual();     // clear wdt reset flag and ensure WDT is known configuration
    ignore_wakeup = false;

    // sleep for the specified number of seconds
    for (uint32_t i = 0; i < seconds;)
    {
        // setup WDT
        WDTCR |= _BV(WDCE) | _BV(WDE); // allow changes, enable WDT
        WDTCR = _BV(WDIE) | WDTO_1S;   // set WDT to interrupt only mode with 1s timeout

        // sleep until WDT interrupt
        set_sleep_mode(SLEEP_MODE_PWR_DOWN); // deepest sleep mode, only WDT can wake up the MCU
        if (!ignore_wakeup) wdt_reset();     // start wdt at zero unless last wakeup was not caused by WDT
        ignore_wakeup = false;
        sei();                               // enable interrupts
        sleep_mode();                        // sleep until WDT interrupt
                                             // after interrupt, CPU continues here
        if (!ignore_wakeup) i++;             // wakeup was not caused by something else, increment i
    }

    // disable interrupts and WDT
    cli();
    wdt_reset();
    disable_actual();
}

void wdt::wakeup_was_not_wdt()
{
    ignore_wakeup = true;
}

void wdt::reset_cpu()
{
    // to reset the cpu, we enable the watchdog timer with a very short timeout.
    // then, we hang in a loop until the watchdog timer resets the CPU for us.

    // note: wdt_enable resets WDTIE, so we don't need to disable it explicitly
    disable_actual();
    wdt_enable(WDTO_15MS);
    while (1)
        ;
}
