#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "util.hpp"
#include "print.hpp"
#include "pulse_counter.hpp"
#include "i2c_device/i2c_device.hpp"

// load enable pin
constexpr uint8_t PIN_LOAD_EN = PB2;

// how long to sleep for in seconds
constexpr uint32_t SLEEP_TIME = 30 * 60;

// pin that low pulses are counted on
// note: must be PB1 since INT0 is fixed to it
// constexpr uint8_t PIN_PULSE_COUNTER = PB1;

// i2c device address
constexpr uint8_t I2C_ADDRESS = 0x50;

// upon receiving this command, the device will power down the load and sleep for SLEEP_TIME
constexpr uint8_t I2C_COMMAND_POWER_DOWN = 0x01;

// read the pulse count
// the number of pulses returned will be subtracted from the total pulse count.
// thus, if the pulse count is larger than 255, multiple reads will be required.
// the pulse count will be reset to 0 after reading completes.
constexpr uint8_t I2C_COMMAND_READ_PULSE_COUNT = 0x03;

/**
 * power down the load and sleep for the specified time
 */
void power_down()
{
    PRINT_PREFIX();
    PRINTLN(F("power_down()"));

    // set all pins to input, no pull-up, to reduce power consumption
    DDRB = 0x00;
    PORTB = 0x00;

    // sleep for reqeusted time
    sleep_for(SLEEP_TIME);

    // reset the CPU
    reset_cpu();
}

bool i2c_address_handler(const uint8_t address)
{
    PRINT_PREFIX();
    PRINT(F("i2c_address_handler called for 0x"));
    PRINTLN(address, HEX);

    return address == I2C_ADDRESS;
}

void i2c_request_handler(const uint8_t address, const bool write, uint8_t &data)
{
    static uint8_t response_data;

    PRINT_PREFIX();
    PRINT(F("i2c_request_handler called for a=0x"));
    PRINT(address, HEX);
    PRINT(F(", w="));
    PRINT(write ? F("0") : F("1"));
    PRINT(F(", data=0x"));
    if (write)
    {
        PRINT(response_data, HEX);
    }
    else
    {
        PRINT(data, HEX);
    }
    PRINTLN();

    if (write)
    {
        // handle command
        switch (data)
        {
        case I2C_COMMAND_POWER_DOWN:
            power_down();
            break;
        case I2C_COMMAND_READ_PULSE_COUNT:
            if (pulse_counter::count > 255)
            {
                response_data = 255;
                pulse_counter::count -= 255;
            }
            else
            {
                response_data = pulse_counter::count;
                pulse_counter::count = 0;
            }
        default:
            break;
        }
    }
    else
    {
        // hopefully we've had a write command to request some data,
        // otherwise this will be invalid;
        data = response_data;
    }
}

int main()
{
    PRINT_BEGIN();
    PRINT_PREFIX();
    PRINTLN();

    // disable interrupts, we don't need them yet
    cli();

    // disable ADC, internal COMP, and WDT to reduce power consumption
    PRINT_PREFIX();
    PRINTLN(F("setting power saving measures"));
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

    // start counting pulses
    pulse_counter::begin();

    // wait for i2c to do something
    PRINT_PREFIX();
    PRINTLN(F("init done, waiting idle"));
    for (;;)
    {
        asm volatile("nop");
    }
    return 0;
}

ISR(PCINT0_vect)
{
    if (i2c::on_pcint0())
        return;
    if (pulse_counter::on_pcint0())
        return;
}
