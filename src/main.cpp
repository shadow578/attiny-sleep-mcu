#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

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

/**
 * required by sleep_for()
 */
EMPTY_INTERRUPT(WDT_vect);

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

/**
 * Wait for the specified number of seconds, spending as much time as possible in sleep mode.
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
 * and https://electronics.stackexchange.com/a/74850
 */
void sleep_for(uint32_t seconds)
{
    cli();                // disable interrupts
    wdt_disable_actual(); // clear wdt reset flag and ensure WDT is known configuration

    // sleep for the specified number of seconds
    for (uint32_t i = 0; i < seconds; i++)
    {
        // setup WDT
        WDTCR |= _BV(WDCE) | _BV(WDE); // allow changes, enable WDT
        WDTCR = _BV(WDIE) | WDTO_1S;   // set WDT to interrupt only mode with 1s timeout

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

/**
 * Reset the CPU.
 */
void reset_cpu()
{
    // to reset the cpu, we enable the watchdog timer with a very short timeout.
    // then, we hang in a loop until the watchdog timer resets the CPU for us.

    // note: wdt_enable resets WDTIE, so we don't need to disable it explicitly
    wdt_disable_actual();
    wdt_enable(WDTO_15MS);
    while (1)
        ;
}

int main()
{
    // disable interrupts, we don't need them yet
    cli();

    // disable ADC, internal COMP, and WDT to reduce power consumption
    wdt_disable_actual();   // disable watchdog timer early in case it's enabled somehow
    ADCSRB &= ~_BV(ACME);   // analog comparator multiplexer enable = 0
    ACSR |= _BV(ACD);       // analog comparator disable = 1
    ADCSRA &= ~_BV(ADEN);   // ADC enable = 0
    power_adc_disable();    // ADC power reduction (PRR.PRADC) = 1
    power_timer0_disable(); // timer0 power reduction (PRR.PRTIM0) = 1
    // power_all_disable();

    // ensure all pins start out as input with no pull-up
    DDRB = 0x00;  // set all pins to input
    PORTB = 0x00; // disable pull-up on all pins

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

    // set all pins to input, no pull-up, to reduce power consumption
    DDRB = 0x00;
    PORTB = 0x00;

    // wait for ~30 minutes
    constexpr uint32_t SLEEP_TIME_ADJUSTED = static_cast<uint32_t>(static_cast<double>(SLEEP_TIME) * SLEEP_TIME_CORRECTION);
    sleep_for(SLEEP_TIME_ADJUSTED);

    // reset the CPU
    reset_cpu();
    return 0;
}
