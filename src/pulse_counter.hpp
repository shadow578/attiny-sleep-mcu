/**
 * count pulses on INT0 (PB1)
 */
#pragma once
#include <avr/io.h>

namespace pulse_counter
{
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

} // namespace pulse_counter
