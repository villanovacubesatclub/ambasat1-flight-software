#ifndef __VoltageSensor__
#define __VoltageSensor__
#include "SensorBase.h"

//
// Voltage Sensor
//

class VoltageSensor : public SensorBase {
private:
    uint8_t _buffer[2];

    int16_t readVccMilliVolts(void) const;
public:
    VoltageSensor();
    virtual ~VoltageSensor();

    virtual void setup(void);
    virtual const uint8_t* getCurrentMeasurementBuffer(void);
    virtual uint8_t getMeasurementBufferSize() const                    { return 2; }
    virtual uint8_t getPort() const                                     { return 1; }
};

#endif