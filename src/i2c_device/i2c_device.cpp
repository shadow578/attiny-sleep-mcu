#include "i2c_device.hpp"
#include <avr/interrupt.h>
#include "../util.hpp"
#include "../print.hpp"

using namespace i2c;

address_handler_t address_handler = nullptr;
request_handler_t request_handler = nullptr;
state_t state = WAIT_FOR_START;

inline bool read_sda() { return PINB & _BV(PIN_SDA); }
inline bool read_scl() { return PINB & _BV(PIN_SCL); }
inline void sda_high()
{
    // input with pull-up
    PORTB |= _BV(PIN_SDA);
    DDRB &= ~_BV(PIN_SDA);
}
inline void sda_low()
{
    // drive low
    PORTB &= ~_BV(PIN_SDA);
    DDRB |= _BV(PIN_SDA);
}

void i2c::begin(const address_handler_t ah, const request_handler_t rh)
{
    PRINT_PREFIX(); 
    PRINTLN(F("i2c::begin"));

    address_handler = ah;
    request_handler = rh;

    // set SDA and SCL pins as input with pull-up
    PORTB |= _BV(PIN_SDA) | _BV(PIN_SCL);
    DDRB &= ~(_BV(PIN_SDA) | _BV(PIN_SCL));

    // enable SDA pin change interrupt
    #if IS_ATTINY13
        PCMSK |= _BV(SDA_PCINT);
        GIMSK |= _BV(PCIE);
    #else
        PCMSK0 |= _BV(SDA_PCINT);
        PCICR |= _BV(PCIE0);
    #endif

    // enable global interrupts
    sei();
}

/**
 * read a byte from the I2C bus
 * @param data byte to read into
 * @return 0 when successful, 1 when STOP condition detected, 2 when START condition detected
 */
uint8_t read_byte(uint8_t &data)
{
    PRINT_PREFIX();
    PRINTLN(F("read_byte"));

    for (uint8_t i = 0; i < 8; i++)
    {
        // wait for SCL to go high
        while (!read_scl())
            ;

        // read the bit
        const bool sdabit = read_sda();
        data = data << 1;
        if (sdabit)
            data |= 1;

        // wait for SCL to go low
        while (read_scl())
        {
            // if SDA changes while SCL is high, this is a
            if (sdabit != read_sda())
            {
                // stop condition (low -> high)
                // start condition (high -> low)
                return sdabit ? 1 : 2;
            }
        }
    }

    PRINT_PREFIX(); 
    PRINT(F("read_byte result 0x"));
    PRINTLN(data, HEX);
    return 0;
}

/**
 * write a byte to the I2C bus
 * @param data byte to write
 */
void write_byte(uint8_t data)
{
    PRINT_PREFIX(); 
    PRINT(F("write_byte 0x"));
    PRINTLN(data, HEX);

    for (uint8_t i = 0; i < 8; i++)
    {
        // set SDA to the bit, MSB first
        if (data & 0x80)
        {
            sda_high();
        }
        else
        {
            sda_low();
        }

        // wait for SCL to go high
        while (!read_scl())
            ;

        // wait for SCL to go low
        while (read_scl())
            ;

        // shift data
        data = data << 1;
    }

    // accept ACK or NACK
    sda_high();
    while (!read_scl())
        ;
    while (read_scl())
        ;
}

/**
 * write a ACK to the I2C bus
 */
void write_ack()
{
    sda_low();
    while (!read_scl())
        ;
    while (read_scl())
        ;
    sda_high();
}

/**
 * pin change on SDA pin
 */
bool i2c::on_pcint0()
{
    // SDA is low and SCL is high?
    if (!(!read_sda() && read_scl()))
    {
        // no, abort
        return false;
    }
    
    // this is a start condition, disable pin change interrupt and reset state
    // we handle the rest synchronously in the interrupt
    #if IS_ATTINY13
        GIMSK &= ~_BV(PCIE);
    #else
        PCICR &= ~_BV(PCIE0);
    #endif
    state = RECEIVE_ADDRESS;

    PRINT_PREFIX();
    PRINTLN(F("i2c state=RECEIVE_ADDRESS (PCINT start)"));

    // wait for SCL to go low
    while (read_scl())
        ;

    // enter state machine loop until we get back to WAIT_FOR_START
    uint8_t address, data;
    while (state != WAIT_FOR_START)
    {
        switch (state)
        {
        case RECEIVE_ADDRESS:
        {
            const uint8_t r = read_byte(address);
            state = HANDLE_ADDRESS;

            // STOP condition?
            if (r == 1)
            {
                state = WAIT_FOR_START;
                PRINT_PREFIX();
                PRINTLN(F("i2c state=WAIT_FOR_START (RECEIVE_ADDRESS STOP received)"));
                break;
            }

            // START condition?
            if (r == 2)
            {
                state = RECEIVE_ADDRESS;
                PRINT_PREFIX();
                PRINTLN(F("i2c state=RECEIVE_ADDRESS (RECEIVE_ADDRESS START received)"));
                break;
            }

            PRINT_PREFIX();
            PRINTLN(F("i2c state=HANDLE_ADDRESS (RECEIVE_ADDRESS ok)"));
            break;
        }
        case HANDLE_ADDRESS:
        {
            const bool handle = address_handler(address & 0xfe);
            if (handle)
            {
                // this device handles the address, ACK
                write_ack();

                // look at the R/W bit to determine next state
                state = (address & 1) ? SEND_DATA : RECEIVE_DATA;
                
                // note: when replacing the above tenary with a if/else, the 
                // binary grows for no reason, even on attiny13 target...
                PRINT_PREFIX();
                PRINT(F("i2c state="));
                PRINT(state == SEND_DATA ? F("SEND_DATA") : F("RECEIVE_DATA"));
                PRINTLN(F(" (HANDLE_ADDRESS)"));
            }
            else
            {
                // we don't handle the address, wait for next start condition
                state = WAIT_FOR_START;
                PRINT_PREFIX();
                PRINTLN(F("i2c state=WAIT_FOR_START (HANDLE_ADDRESS not handled)"));
            }

            break;
        }
        case RECEIVE_DATA:
        {
            const uint8_t r = read_byte(data);
            state = WAIT_FOR_START;

            // STOP condition?
            if (r == 1)
            {
                state = WAIT_FOR_START;
                PRINT_PREFIX(); 
                PRINTLN(F("i2c state=WAIT_FOR_START (RECEIVE_DATA STOP received)"));
                break;
            }

            // START condition?
            if (r == 2)
            {
                state = RECEIVE_ADDRESS;
                PRINT_PREFIX(); 
                PRINTLN(F("i2c state=RECEIVE_ADDRESS (RECEIVE_DATA START received)"));
                break;
            }

            // data is for us, send ACK then handle it
            write_ack();
            request_handler(address, true, data);
            
            PRINT_PREFIX(); 
            PRINTLN(F("i2c state=WAIT_FOR_START (RECEIVE_DATA done)"));
            break;
        }
        case SEND_DATA:
        {
            request_handler(address, false, data);
            state = WAIT_FOR_START;

            write_byte(data);

            PRINT_PREFIX(); 
            PRINTLN(F("i2c state=WAIT_FOR_START (SEND_DATA done)"));
            break;
        }
        default:
        {
            state = WAIT_FOR_START;
            PRINT_PREFIX(); 
            PRINTLN(F("i2c state=WAIT_FOR_START (invalid state)"));
            break;
        }
        }
    }

    PRINT_PREFIX(); 
    PRINTLN(F("i2c PCINT done"));

    // re-enable pin change interrupt
    // we are WAIT_FOR_START again
    #if IS_ATTINY13
        GIMSK |= _BV(PCIE);
    #else
        PCICR |= _BV(PCIE0);
    #endif

    return true;
}
