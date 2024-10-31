/**
 * Example showing how to interact with the sleep mcu over i2c.
 */
#include <Arduino.h>
#include <Wire.h>

constexpr uint8_t I2C_ADDRESS = 0x50;
constexpr uint8_t I2C_COMMAND_POWER_DOWN = 0x01;
constexpr uint8_t I2C_COMMAND_READ_PULSE_COUNT = 0x03;

uint8_t send_command(const uint8_t cmd, const bool read_back = false)
{
    Serial.print("[I2C] >> 0x");
    Serial.println(cmd, HEX);

    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(cmd);
    Wire.endTransmission();

    if (!read_back)
    {
        return 0;
    }

    delay(1); // small delay to give device time for processing

    Wire.requestFrom(I2C_ADDRESS, static_cast<uint8_t>(1));
    const uint8_t b = Wire.read();

    Serial.print("[I2C] << 0x");
    Serial.println(b, HEX);
    return b;
}

uint32_t read_pulse_count()
{
    uint32_t pulse_count = 0;
    uint8_t cnt;
    do {
        cnt = send_command(I2C_COMMAND_READ_PULSE_COUNT, true);
        pulse_count += cnt;
    } while(cnt > 0);

    return pulse_count;
}

void setup()
{
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(1000); // slow down i2c clock to 1 kHz

    Serial.print("pulse count: ");
    Serial.println(read_pulse_count());

    Serial.println("power down now");
    send_command(I2C_COMMAND_POWER_DOWN);

    Serial.println("done!");
}

void loop() {}
