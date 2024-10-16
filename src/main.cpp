#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <util/delay.h>

// pin definitions
constexpr uint8_t PIN_PWR_DOWN = PB0;
constexpr uint8_t PIN_LOAD_EN = PB1;

// for how long the device should sleep before turning on the load again
constexpr uint32_t SLEEP_TIME = 30 * 60; // seconds

// _delay_ms() only works ok for values < 262.14 ms, so we call it multiple times in a loop
constexpr uint32_t SLEEP_TIME_TICK_MS = 100;                                        // how much time to wait in each loop iteration
constexpr uint32_t SLEEP_TIME_TICKS = ((SLEEP_TIME * 1000UL) / SLEEP_TIME_TICK_MS); // number of iterations
static_assert(SLEEP_TIME_TICKS < 0xFFFF, "SLEEP_TIME_TICKS must be less than 0xFFFF");

int main()
{
    // disable interrupts, we don't need them
    cli();

    // disable ADC, internal COMP, and WDT to reduce power consumption
    ADCSRA &= ~_BV(ADEN);   // ADC enable = 0
    power_adc_disable();    // ADC power reduction (PRR.PRADC) = 1
    ADCSRB &= ~_BV(ACME);   // analog comparator multiplexer enable = 0
    ACSR |= _BV(ACD);       // analog comparator disable = 1
    WDTCR &= ~_BV(WDE);     // disable watchdog timer
    wdt_disable();          // "
    power_timer0_disable(); // timer0 power reduction (PRR.PRTIM0) = 1

    // ensure all pins start out as input with no pull-up
    DDRB = 0x00;
    PORTB = 0x00;

    // set power down pin to input
    // DDRB &= ~_BV(PIN_PWR_DOWN);

    // set LOAD_EN to output, set HIGH
    DDRB |= _BV(PIN_LOAD_EN);
    PORTB |= _BV(PIN_LOAD_EN);

    // wait until power down pin is HIGH
    while (!(PINB & _BV(PIN_PWR_DOWN)))
        ;

    // set all pins to input, no pull-up, to reduce power consumption
    DDRB = 0x00;  // set all pins to input
    PORTB = 0x00; // disable pull-up on all pins

    // TODO: more power consumption reduction possible ?
    // note: cannot enter any of the sleep mode, since they all disable the CPU.

    // wait for ~30 minutes
    for (uint32_t i = 0; i < SLEEP_TIME_TICKS; i++)
    {
        _delay_ms(SLEEP_TIME_TICK_MS);
    }

    // reset the MCU using watchdog timer
    // for this, enable the watchdog timer with a short timeout and then hang until it resets the MCU.
    // since the WDTs oscillator is not very accurate, the actual timeout may vary.
    // using 250ms should fall somewhere between 100ms and 750ms.
    wdt_enable(WDTO_250MS);
    while (1)
        ;

    return 0;
}
