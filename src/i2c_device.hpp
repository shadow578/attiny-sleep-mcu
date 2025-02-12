/**
 * I2C device module, based on TinyWireS library but adapted to work on bare AVR
 */

#pragma once
#include <stdint.h>

namespace i2c_device
{
    void begin(uint8_t address);

    void write(uint8_t data);

    uint8_t available();
    uint8_t read();

    /**
     * @brief set callback for write to device
     * @param callback function to call when data is received. parameter is number of bytes available
     */
    void on_write(void (*callback)(uint8_t));

    /**
     * @brief set callback for read from device
     * @param callback function to call when data is requested
     */
    void on_read(void (*callback)(void));
} // namespace i2c_device
