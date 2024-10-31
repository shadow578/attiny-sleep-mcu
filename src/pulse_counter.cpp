#include "pulse_counter.hpp"
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "util.hpp"
#include "print.hpp"

using namespace pulse_counter;

volatile uint16_t pulse_counter::count = 0;

void pulse_counter::begin()
{
    PRINT_PREFIX();
    PRINTLN(F("pulse_counter::begin()"));

    // set pin as input with pull-up resistor
    DDRB &= ~_BV(PIN_PULSE_COUNTER);
    PORTB |= _BV(PIN_PULSE_COUNTER);

    // enable pin change interrupt
    #if IS_ATTINY13
        PCMSK |= _BV(PULSE_COUNTER_PCINT);
        GIMSK |= _BV(PCIE);
    #else
        PCMSK0 |= _BV(PULSE_COUNTER_PCINT);
        PCICR |= _BV(PCIE0);
    #endif

    // enable global interrupts
    sei();
}

bool pulse_counter::on_pcint0()
{
    // is counting pin low?
    if (PINB & _BV(PIN_PULSE_COUNTER))
    {
        // no, abort
        return false;
    }

    // increment pulse count
    count++;

    // re-enter sleep mode if SE bit is set
    if (MCUCR & _BV(SE))
    {
        sleep_cpu();
    }

    return true;
}
