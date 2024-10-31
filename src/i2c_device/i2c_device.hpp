/**
 * simple I2C device library for the attiny13.
 * note that SDA and SCL pins must both reside on PORT B.
 * 
 * based on https://github.com/leonhiem/i2c_attiny13/blob/main/mod/main.c
 */

#pragma once
#include <avr/io.h>

namespace i2c
{
    #if !defined(CONF_PIN_SDA)
        #define CONF_PIN_SDA PB3
    #endif

    #if !defined(CONF_PIN_SCL)
        #define CONF_PIN_SCL PB4
    #endif

    #if !defined(CONF_SDA_PCINT)
        #if CONF_PIN_SDA == PB0
            #define CONF_SDA_PCINT PCINT0
        #elif CONF_PIN_SDA == PB1
            #define CONF_SDA_PCINT PCINT1
        #elif CONF_PIN_SDA == PB2
            #define CONF_SDA_PCINT PCINT2
        #elif CONF_PIN_SDA == PB3
            #define CONF_SDA_PCINT PCINT3
        #elif CONF_PIN_SDA == PB4
            #define CONF_SDA_PCINT PCINT4
        #elif CONF_PIN_SDA == PB5
            #define CONF_SDA_PCINT PCINT5
        #else
            #error "cannot auto-configure CONF_SDA_PCINT! please specify it manually!"
        #endif
    #endif

    /**
     * I2C SDA pin bit number
     */
    constexpr uint8_t PIN_SDA = CONF_PIN_SDA;

    /**
     * I2C SCL pin bit number
     */
    constexpr uint8_t PIN_SCL = CONF_PIN_SCL;

    /**
     * I2C SCL pin change interrupt bit number
     */
    constexpr uint8_t SDA_PCINT = CONF_SDA_PCINT;

    /**
     * I2C state machine states
     */
    enum state_t : uint8_t
    {
        /**
         * waiting for start condition
         */
        WAIT_FOR_START,

        /**
         * receiving address byte
         */
        RECEIVE_ADDRESS,

        /**
         * check the address, send ACK if handled
         */
        HANDLE_ADDRESS,

        /**
         * receiving data byte
         */
        RECEIVE_DATA,

        /**
         * sending data byte
         */
        SEND_DATA,
    };

    /**
     * I2C request handler
     * @param address I2C address
     * @return whether the request was handled by this device (address matches)
     */
    typedef bool (*address_handler_t)(const uint8_t address);

    /**
     * I2C data handler
     * @param address I2C address the request was sent to.
     *                pre-validated by address_handler
     * @param write if false, the request is a read request.
     *              if true, the request is a write request
     * @param data data byte.
     *             for a write request, this is the byte received.
     *             for a read request, this is the byte to send
     */
    typedef void (*request_handler_t)(const uint8_t address, const bool write, uint8_t &data);

    /**
     * initialize I2C device
     * @param address_handler I2C address handler
     * @param request_handler I2C read or write request handler
     * @note global interrupts are enabled by this function and must remain enabled to receive I2C requests
     */
    void begin(const address_handler_t address_handler, const request_handler_t request_handler);

    /**
     * i2c::begin sets up a pin change interrupt on the SDA pin. 
     * it's the users responsibility to call this function from the PCINT0 interrupt handler.
     * @return did this function handle the interrupt?
     * @note extra calls (e.g. because other functions also use PCINT0) are permitted
     */
    bool on_pcint0();

} // namespace i2c
