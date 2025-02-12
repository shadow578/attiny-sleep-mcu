#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "i2c_device.hpp"
#include "lp.hpp"
#include "wdt.hpp"


// pin definitions
constexpr uint8_t PIN_LOAD_EN = PB4;  // pin attached to load mosfet gate
constexpr uint8_t PIN_COUNTER = PB1;  // pin that pulses are counted on, e.g. for rain meter

// i2c constants
constexpr uint8_t I2C_DEVICE_ADDRESS = 0x64;

enum i2c_command : uint8_t
{
    COMMAND_SLEEP = 0x01,
    COMMAND_SET_SLEEP_TIME = 0x02,
    COMMAND_GET_COUNTER_VALUE = 0x03,
    COMMAND_RESET_COUNTER = 0x04
};

// for how long the device should sleep before turning on the load again
// uses the WDT for timing, so will be fairly inaccurate
static uint32_t sleep_time = (30 * 60); // 30 minutes

// signal to go to sleep now
static bool go_sleep = false;

// counter for PIN_COUNTER falling edges
static volatile uint32_t counter = 0;

// last i2c command received
static volatile uint8_t i2c_command = 0;

void on_i2c_write(const uint8_t len)
{
    go_sleep = true;

    if (len < 1) return;

    i2c_command = i2c_device::read();
    switch (i2c_command)
    {
    case COMMAND_SLEEP:
    {
        i2c_device::write(0x00);
        go_sleep = true;
        break;
    }
    case COMMAND_SET_SLEEP_TIME:
    {
        // need at least 4 bytes + command byte
        if (len < 5) return;

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
        // send counter value in on_i2c_read() function
        break;
    }
    case COMMAND_RESET_COUNTER:
    {
        counter = 0;
    }
    default:
        break;
    }
}

void on_i2c_read()
{
    go_sleep = true;

    switch(i2c_command)
    {
    case COMMAND_GET_COUNTER_VALUE:
    {
        i2c_device::write((counter << 24) & 0xff);
        i2c_device::write((counter << 16) & 0xff);
        i2c_device::write((counter << 8) & 0xff);
        i2c_device::write(counter & 0xff);
    }
    default:
        break;
    }

    // reset command
    i2c_command = 0;
}

ISR(PCINT0_vect)
{
    // count PIN_COUNTER on falling edge
    if ((PORTB & _BV(PIN_COUNTER)) == 0)
    {
        counter++;
    }

    // signal wdt sleep routine that this was not a wdt interrupt
    wdt::wakeup_was_not_wdt();
}

void loop()
{
    // disable all feasible peripherals
    lp::power_down_all();

    // ensure all pins start out as input with no pull-up
    lp::reset_gpio();

    // setup i2c device
    i2c_device::begin(I2C_DEVICE_ADDRESS);
    i2c_device::on_write(on_i2c_write);
    i2c_device::on_read(on_i2c_read);

    // set LOAD_EN to output, set HIGH to turn on load
    DDRB |= _BV(PIN_LOAD_EN);
    PORTB |= _BV(PIN_LOAD_EN);

    // set counter pin to input with pullup
    // (redundant) DDRB &= ~_BV(PIN_COUNTER); 
    PORTB |= _BV(PIN_COUNTER);

    // enable PCINT
    sei();
    GIMSK |= _BV(PCIE);
    PCMSK = _BV(PIN_COUNTER); // note: PCINTn and PBn happen to align on attiny85, so this is right

    // run idle while not sleeping
    uint32_t c = 0;
    while(!go_sleep)
    {
        _delay_ms(10);
        c++;

        // after 30 seconds
        //if (c > 3000) go_sleep = true;
    }

    // re-disable all feasible peripherals again, to make sure
    // e.g. USI will have been re-enabled
    lp::power_down_all();

    // reset all pins for low-power
    // this also turns off the load
    lp::reset_gpio();

    // wait for ~30 minutes
    wdt::sleep_for(sleep_time);
}

int main()
{
    for(;;) loop();
    return 0;
}
