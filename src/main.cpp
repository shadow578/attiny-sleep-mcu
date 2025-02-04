#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "i2c_device.hpp"
#include "lp.hpp"
#include "wdt.hpp"


// pin definitions
constexpr uint8_t PIN_LOAD_EN = PB1;  // pin attached to load mosfet gate

// i2c constants
constexpr uint8_t I2C_DEVICE_ADDRESS = 0x64;

enum i2c_command : uint8_t
{
    COMMAND_SLEEP = 0x01,
    COMMAND_SET_SLEEP_TIME = 0x02,
    COMMAND_GET_COUNTER_VALUE = 0x03
};

// for how long the device should sleep before turning on the load again
// uses the WDT for timing, so will be fairly inaccurate
static uint32_t sleep_time = (30 * 60); // 30 minutes


// signal to go to sleep now
static bool go_sleep = false;

void on_i2c_request()
{
    const uint8_t command = i2c_device::read();
    switch (command)
    {
    case COMMAND_SLEEP:
    {
        i2c_device::write(0x00);
        break;
    }
    case COMMAND_SET_SLEEP_TIME:
    {
        uint32_t t = 0;
        t += i2c_device::read();
        t <<= 8;
        t += i2c_device::read();
        t <<= 8;
        t += i2c_device::read();
        t <<= 8;
        t += i2c_device::read();
        t <<= 8;

        sleep_time = t;
        break;
    }
    case COMMAND_GET_COUNTER_VALUE:
    {
         // TODO: actually send meaningful data
        i2c_device::write(0xde);
        i2c_device::write(0xad);
        i2c_device::write(0xbe);
        i2c_device::write(0xef);
        break;
    }
    default:
        break;
    }
}

int main()
{
    // disable interrupts, we don't need them yet
    cli();

    // disable all feasible peripherals
    lp::power_down_all();

    // ensure all pins start out as input with no pull-up
    lp::reset_gpio();

    // setup i2c device
    i2c_device::begin(I2C_DEVICE_ADDRESS);
    i2c_device::on_request(on_i2c_request);

    // set LOAD_EN to output, set HIGH to turn on load
    DDRB |= _BV(PIN_LOAD_EN);
    PORTB |= _BV(PIN_LOAD_EN);

    // run idle while not sleeping
    while(!go_sleep)
        _delay_ms(10);

    // re-disable all feasible peripherals again, to make sure
    // e.g. USI will have been re-enabled
    lp::power_down_all();

    // reset all pins for low-power
    // this also turns off the load
    lp::reset_gpio();

    // wait for ~30 minutes
    wdt::sleep_for(sleep_time);

    // reset the CPU
    wdt::reset_cpu();
    return 0;
}
