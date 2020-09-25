#ifndef __STS21Sensor__
#define __STS21Sensor__
#include "SensorBase.h"

class PersistedConfiguration;

class STS21Sensor : public SensorBase
{
private:
    uint8_t _buffer[5];
    
    bool begin(void);
    void reset(void);
    
protected:
    // The SHT21 has no concept of register address autoincrement
    virtual uint8_t i2cAutoIncrementBit(void) const             { return 0; } 
    virtual uint8_t i2cDeviceAddress(void) const                { return 0x4A; }
public:
    STS21Sensor(PersistedConfiguration& config);
    virtual ~STS21Sensor();

    virtual void setup(void);
    virtual bool isActive(void) const;
    virtual const uint8_t* getCurrentMeasurementBuffer(void);
    virtual uint8_t getMeasurementBufferSize() const            { return 5; }
    virtual uint8_t getPort() const                             { return 4; }

};

#endif //__STS21Sensor__