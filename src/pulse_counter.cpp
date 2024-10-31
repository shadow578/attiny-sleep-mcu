#include "pulse_counter.hpp"
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "util.hpp"

using namespace pulse_counter;

volatile uint16_t pulse_counter::count = 0;

void pulse_counter::begin()
{
    // set PB1 as input with pull-up resistor
    DDRB &= ~_BV(PB1); // TODO on non-attiny13, this pin doesn't match!
    PORTB |= _BV(PB1);

    // enable interrupt on low level,
    // since only INT0 level interrupts can wake the MCU from sleep
    MCUCR &= ~_BV(ISC00) | ~_BV(ISC01); // ICS00 = 0, ICS01 = 0

    // enable INT0
    #if IS_ATTINY13
        GIMSK |= _BV(INT0);
    #else
        #warning "atmega code missing"
    #endif

    // enable global interrupts
    sei();
}

// TODO: use a PCINT instead of INT0 for additional flexibility
ISR(INT0_vect)
{
    // increment pulse count
    count++;

    // re-enter sleep mode if SE bit is set
    if (MCUCR & _BV(SE))
    {
        sleep_cpu();
    }
}
