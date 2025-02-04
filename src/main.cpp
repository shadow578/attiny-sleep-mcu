#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "wdt.hpp"
#include "lp.hpp"

// pin definitions
constexpr uint8_t PIN_PWR_DOWN = PB2; // pin to indicate that the load should be powered down
constexpr uint8_t PIN_LOAD_EN = PB1;  // pin attached to load mosfet gate

// PIN_PWR_DOWN state when asserted (= should power down the load)
constexpr bool PWR_DOWN_ASSERTED_STATE = false; // pulled high; active low

// for how long the device should sleep before turning on the load again
// uses the WDT for timing, so will be fairly inaccurate even with correction factor applied.
// constant is in seconds
constexpr uint32_t SLEEP_TIME = (30 * 60); // 30 minutes

// correction factor for sleep time.
// calibrate by measuring actual sleep time, then set this to (desired sleep time / actual sleep time)
constexpr double SLEEP_TIME_CORRECTION = (30.0 / 37.0);

int main()
{
    // disable interrupts, we don't need them yet
    cli();

    // disable all feasible peripherals
    lp::power_down_all();

    // ensure all pins start out as input with no pull-up
    lp::reset_gpio();

    // set power down pin to input
    // DDRB &= ~_BV(PIN_PWR_DOWN); // redundant: all pins are input at this point

    // set LOAD_EN to output, set HIGH
    DDRB |= _BV(PIN_LOAD_EN);
    PORTB |= _BV(PIN_LOAD_EN);

    // wait until power down pin is asserted
    if (PWR_DOWN_ASSERTED_STATE)
    {
        // active high, wait until high
        while (!(PINB & _BV(PIN_PWR_DOWN)))
            _delay_ms(10);
    }
    else
    {
        // active low, wait until low
        while (PINB & _BV(PIN_PWR_DOWN))
            _delay_ms(10);
    }

    // reset all pins for low-power
    lp::reset_gpio();

    // wait for ~30 minutes
    constexpr uint32_t SLEEP_TIME_ADJUSTED = static_cast<uint32_t>(static_cast<double>(SLEEP_TIME) * SLEEP_TIME_CORRECTION);
    wdt::sleep_for(SLEEP_TIME_ADJUSTED);

    // reset the CPU
    wdt::reset_cpu();
    return 0;
}
