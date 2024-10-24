#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "util.hpp"
#include "i2c_device/i2c_device.hpp"

// load enable pin
constexpr uint8_t PIN_LOAD_EN = PB2;

// for how long the device should sleep before turning on the load again
// uses the WDT for timing, so will be fairly inaccurate.
constexpr uint32_t SLEEP_TIME = 30 * 60; // seconds, rounded up to nearest 8s

// i2c configuration
constexpr uint8_t I2C_ADDRESS = 0x50;
constexpr uint8_t I2C_COMMAND_POWER_DOWN = 0x01; // upon receiving this command, the device will power down the load and sleep for SLEEP_TIME

/**
 * power down the load and sleep for the specified time
 */
void power_down()
{
    // set all pins to input, no pull-up, to reduce power consumption
    DDRB = 0x00;
    PORTB = 0x00;

    // sleep for ~30 minutes
    sleep_for(SLEEP_TIME);

    // reset the CPU
    reset_cpu();
}

bool i2c_address_handler(const uint8_t address)
{
    return address == I2C_ADDRESS;
}

void i2c_request_handler(const uint8_t address, const bool write, uint8_t &data)
{
    if (write)
    {
        switch (data)
        {
        case I2C_COMMAND_POWER_DOWN:
            power_down();
            break;

        default:
            break;
        }
    }
}

int main()
{
    // disable interrupts, we don't need them yet
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

    // set LOAD_EN to output, set HIGH to power the load
    DDRB |= _BV(PIN_LOAD_EN);
    PORTB |= _BV(PIN_LOAD_EN);

    // start i2c
    // this enables interrupts
    i2c::begin(i2c_address_handler, i2c_request_handler);

    // wait for i2c to do something
    for (;;)
    {
        asm volatile("nop");
    }
    return 0;
}
