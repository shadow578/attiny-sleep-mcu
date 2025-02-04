extern "C" 
{
    #include <utility/USI_TWI_Slave.h>
}

#include "i2c_device.hpp"


void i2c_device::begin(uint8_t address)
{
    usiTwiSlaveInit(address);
}

void i2c_device::write(uint8_t data)
{
    usiTwiTransmitByte(data);
}
    
uint8_t i2c_device::available()
{
    return usiTwiAmountDataInReceiveBuffer();
}

uint8_t i2c_device::read()
{
    return usiTwiReceiveByte();
}

void i2c_device::on_receive(void (*callback)(uint8_t))
{
    usi_onReceiverPtr = callback;
}

void i2c_device::on_request(void (*callback)(void))
{
    usi_onRequestPtr = callback;
}
