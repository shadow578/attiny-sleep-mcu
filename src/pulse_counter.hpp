/**
 * count pulses using a pin change interrupt on PORT B.
 */
#pragma once
#include <avr/io.h>

namespace pulse_counter
{
    #if !defined(CONF_PIN_PULSE_COUNTER)
        #define CONF_PIN_PULSE_COUNTER PB1
    #endif

    #if !defined(CONF_PULSE_COUNTER_PCINT)
        #if CONF_PIN_PULSE_COUNTER == PB0
            #define CONF_PULSE_COUNTER_PCINT PCINT0
        #elif CONF_PIN_PULSE_COUNTER == PB1
            #define CONF_PULSE_COUNTER_PCINT PCINT1
        #elif CONF_PIN_PULSE_COUNTER == PB2
            #define CONF_PULSE_COUNTER_PCINT PCINT2
        #elif CONF_PIN_PULSE_COUNTER == PB3
            #define CONF_PULSE_COUNTER_PCINT PCINT3
        #elif CONF_PIN_PULSE_COUNTER == PB4
            #define CONF_PULSE_COUNTER_PCINT PCINT4
        #elif CONF_PIN_PULSE_COUNTER == PB5
            #define CONF_PULSE_COUNTER_PCINT PCINT5
        #else
            #error "cannot auto-configure CONF_SDA_PCINT! please specify it manually!"
        #endif
    #endif

    /**
     * pulse counter pin bit number
     */
    constexpr uint8_t PIN_PULSE_COUNTER = CONF_PIN_PULSE_COUNTER;

    /**
     * pulse counter pin change interrupt bit number
     */
    constexpr uint8_t PULSE_COUNTER_PCINT = CONF_PULSE_COUNTER_PCINT;

    /**
     * current pulse count
     */
    extern volatile uint16_t count;

    /**
     * start counting pulses on PB1.
     * @note this will enable global interrupts.
     *       pulses are only counted while interrupts remain enabled.
     *
     * @note pulses continue to be counted even if the MCU is in sleep mode.
     *       sleep is resumed automatically.
     */
    void begin();

    /**
     * pulse_counter::begin sets up a pin change interrupt on the counting pin. 
     * it's the users responsibility to call this function from the PCINT0 interrupt handler.
     * @return did this function handle the interrupt?
     * @note extra calls (e.g. because other functions also use PCINT0) are permitted
     */
    bool on_pcint0();

} // namespace pulse_counter
