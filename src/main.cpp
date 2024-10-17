#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

// pin definitions
constexpr uint8_t PIN_PWR_DOWN = PB0;
constexpr uint8_t PIN_LOAD_EN = PB1;

// for how long the device should sleep before turning on the load again
// uses the WDT for timing, so will be fairly inaccurate.
constexpr uint32_t SLEEP_TIME = 30 * 60; // seconds, rounded up to nearest 8s

EMPTY_INTERRUPT(WDT_vect);

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
void wait_for(const uint32_t seconds)
{
    cli();

    // enable the watchdog timer with a timeout of 8s, in interrupt mode
    wdt_reset();
    MCUSR &= ~_BV(WDRF);          // clear wdt reset flag
    WDTCR = _BV(WDE) | _BV(WDCE); // enable WDT
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

/**
 * Reset the CPU.
 */
void reset_cpu()
{
    // to reset the cpu, we enable the watchdog timer with a very short timeout.
    // then, we hang in a loop until the watchdog timer resets the CPU for us.

    // note: wdt_enable resets WDTIE, so we don't need to disable it explicitly
    wdt_enable(WDTO_15MS);
    while (1)
        ;
}

int main()
{
    // disable interrupts, we don't need them
    cli();

    // disable ADC, internal COMP, and WDT to reduce power consumption
    ADCSRB &= ~_BV(ACME);   // analog comparator multiplexer enable = 0
    ACSR |= _BV(ACD);       // analog comparator disable = 1
    ADCSRA &= ~_BV(ADEN);   // ADC enable = 0
    power_adc_disable();    // ADC power reduction (PRR.PRADC) = 1
    wdt_disable();          // disable watchdog timer
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

    // wait until power down pin is HIGH
    while (!(PINB & _BV(PIN_PWR_DOWN)))
        ;

    // set all pins to input, no pull-up, to reduce power consumption
    DDRB = 0x00;
    PORTB = 0x00;

    // wait for ~30 minutes
    wait_for(SLEEP_TIME);

    // reset the CPU
    reset_cpu();
    return 0;
}
