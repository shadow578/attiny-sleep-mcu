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

    void on_receive(void (*callback)(uint8_t));
    void on_request(void (*callback)(void));
} // namespace i2c_device
