/**
 * simple I2C device library for the attiny13.
 *
 * based on https://github.com/leonhiem/i2c_attiny13/blob/main/mod/main.c
 */

#pragma once
#include <avr/io.h>

namespace i2c
{
    /**
     * I2C SDA pin bit number
     */
    constexpr uint8_t PIN_SDA = PB1;

    /**
     * I2C SCL pin bit number
     */
    constexpr uint8_t PIN_SCL = PB0;

    /**
     * I2C SCL pin change interrupt bit number
     */
    constexpr uint8_t SDA_PCINT = PCINT1;

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

}; // namespace i2c
