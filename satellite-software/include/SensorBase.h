#ifndef __SensorBase__
#define __SensorBase__
#include <Arduino.h>

class SensorBase {
    static bool _isI2CSetUp;

public:
    SensorBase();
    virtual ~SensorBase();

    virtual void setup(void)    {}

    // This functions is expected to get a current sensor measurement
    // then fill a byte buffer that will be transmitted via the 
    // LoRaWAN radio. It is the sensor's responsibility to define the
    // the buffer format for it's measurement. 
    // IMPORTANT: Returns NULL if sensor read was not successful.
    virtual const uint8_t* getCurrentMeasurementBuffer(void) = 0;

    virtual uint8_t getMeasurementBufferSize() const = 0;

    //
    // I2C methods
    //
    int writeRegister(uint8_t slaveAddress, uint8_t address, uint8_t value);
    int readRegister(uint8_t slaveAddress, uint8_t address);
    int readRegisters(uint8_t slaveAddress, uint8_t address, uint8_t* data, size_t length);
};



#endif // __SensorBase__