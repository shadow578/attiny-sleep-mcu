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
constexpr uint8_t I2C_COMMAND_SLEEP = 0x01;
constexpr uint8_t I2C_COMMAND_GET_COUNT = 0x02;

// for how long the device should sleep before turning on the load again
// uses the WDT for timing, so will be fairly inaccurate even with correction factor applied.
// constant is in seconds
constexpr uint32_t SLEEP_TIME = (30 * 60); // 30 minutes

// correction factor for sleep time.
// calibrate by measuring actual sleep time, then set this to (desired sleep time / actual sleep time)
constexpr double SLEEP_TIME_CORRECTION = (30.0 / 37.0);

static bool go_sleep = false;

void on_i2c_request()
{
    const uint8_t command = i2c_device::read();
    switch (command)
    {
    case I2C_COMMAND_SLEEP:
        i2c_device::write(0x00);
        break;
    case I2C_COMMAND_GET_COUNT:
        // TODO: actually send meaningful data
        i2c_device::write(0xde);
        i2c_device::write(0xad);
        i2c_device::write(0xbe);
        i2c_device::write(0xef);
        break;
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

    // reset all pins for low-power
    // this also turns off the load
    lp::reset_gpio();

    // wait for ~30 minutes
    constexpr uint32_t SLEEP_TIME_ADJUSTED = static_cast<uint32_t>(static_cast<double>(SLEEP_TIME) * SLEEP_TIME_CORRECTION);
    wdt::sleep_for(SLEEP_TIME_ADJUSTED);

    // reset the CPU
    wdt::reset_cpu();
    return 0;
}
