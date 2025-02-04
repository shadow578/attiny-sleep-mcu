#include <avr/io.h>
#include <avr/power.h>

#include "lp.hpp"
#include "wdt.hpp"

void lp::power_down_all()
{
    wdt::disable_actual();
    ADCSRB &= ~_BV(ACME);   // analog comparator multiplexer enable = 0
    ACSR |= _BV(ACD);       // analog comparator disable = 1
    ADCSRA &= ~_BV(ADEN);   // ADC enable = 0
    power_adc_disable();    // ADC power reduction (PRR.PRADC) = 1
    power_timer0_disable(); // timer0 power reduction (PRR.PRTIM0) = 1
    // power_all_disable();
}

void lp::reset_gpio()
{
    DDRB = 0x00;
    PORTB = 0x00;
}
