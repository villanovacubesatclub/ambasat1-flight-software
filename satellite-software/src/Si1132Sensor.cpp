#include <Arduino.h>
#include "Si1132Sensor.h"

#define SI1132_ADDRESS      0x60

#define SI1132_PART_ID_REG          0x00
#define SI1132_REV_ID_REG           0x01
#define SI1132_SEQ_ID_REG           0x02
#define SI1132_HW_KEY_REG           0x07
#define SI1132_MEAS_RATE0_REG       0x08
#define SI1132_MEAS_RATE1_REG       0x09
#define SI1132_UCOEF0_REG           0x13
#define SI1132_UCOEF1_REG           0x14
#define SI1132_UCOEF2_REG           0x15
#define SI1132_UCOEF3_REG           0x16
#define SI1132_PARAM_WR_REG         0x17
#define SI1132_COMMAND_REG          0x18
#define SI1132_RESPONSE_REG         0x20
#define SI1132_ALS_VIS_DATA0_REG    0x22
#define SI1132_ALS_VIS_DATA1_REG    0x23
#define SI1132_ALS_IR_DATA0_REG     0x24
#define SI1132_ALS_IR_DATA1_REG     0x25
#define SI1132_AUX_DATA0_REG        0x2C
#define SI1132_AUX_DATA1_REG        0x2D
#define SI1132_PARAM_RD_REG         0x2E
#define SI1132_CHIP_STAT_REG        0x30

#define SI1132_CMD_RESET        0x01
#define SI1132_CMD_ALS_FORCE    0x06
#define SI1132_CMD_ALS_AUTO     0x0E
#define SI1132_CMD_PARAM_QUERY  0x80
#define SI1132_CMD_PARAM_SET    0xA0

#define SI1132_PARAM_CHLIST                 0x01
#define SI1132_PARAM_ALS_IR_ADCMUX          0x0E
#define SI1132_PARAM_ALS_VIS_ADC_COUNTER    0x10
#define SI1132_PARAM_ALS_VIS_ADC_GAIN       0x11
#define SI1132_PARAM_ALS_VIS_ADC_MISC       0x12
#define SI1132_PARAM_ALS_IR_ADC_COUNTER     0x1D
#define SI1132_PARAM_ALS_IR_ADC_GAIN        0x1E
#define SI1132_PARAM_ALS_IR_ADC_MISC        0x1F
#define SI1132_PARAM_

Si1132Sensor::Si1132Sensor()
    :   _MEAS_RATE0(SI1132_MEAS_RATE0_REG),
        _MEAS_RATE1(SI1132_MEAS_RATE1_REG)
{

    if (!begin())
    {
        Serial.println(F("Oops ... unable to initialize the Si1132"));
        while (1);
    }
}

Si1132Sensor::~Si1132Sensor()
{

}

bool Si1132Sensor::begin(void)
{
    uint8_t device_id = readRegister(SI1132_ADDRESS, SI1132_PART_ID_REG);

    if (device_id != 0x32) {
        Serial.print(F("ERROR when initializing Si1132: device_id = 0x"));
        Serial.print(device_id, HEX);
        Serial.print(F("\n"));
        return false;
    }
    uint8_t revision_id = readRegister(SI1132_ADDRESS, SI1132_REV_ID_REG);
    uint8_t sequence_id = readRegister(SI1132_ADDRESS, SI1132_SEQ_ID_REG);
    Serial.print(F("Found Si1132 UV Light Sensor with revision ID = "));
    Serial.print(revision_id);
    Serial.print(F(", sequence ID = "));
    Serial.print(sequence_id);
    Serial.print(F("\n"));

    if (sequence_id == 0x01) {
        // deal with a software bug in sequence 0x01 models
        _MEAS_RATE0 = 0x0A;
        _MEAS_RATE1 = 0x08;
    }
    return true;
}

void Si1132Sensor::setup(void)
{
 
    // sendCommand(SI1132_CMD_RESET);
    // delay(10);

    // set UCOEF[0:3] to the default values
    writeRegister(SI1132_ADDRESS, SI1132_UCOEF0_REG, 0x7B);
    writeRegister(SI1132_ADDRESS, SI1132_UCOEF1_REG, 0x6B);
    writeRegister(SI1132_ADDRESS, SI1132_UCOEF2_REG, 0x01);
    writeRegister(SI1132_ADDRESS, SI1132_UCOEF3_REG, 0x00);
    // writeRegister(SI1132_ADDRESS, SI1132_UCOEF0_REG, 0x29);
    // writeRegister(SI1132_ADDRESS, SI1132_UCOEF1_REG, 0x89);
    // writeRegister(SI1132_ADDRESS, SI1132_UCOEF2_REG, 0x02);
    // writeRegister(SI1132_ADDRESS, SI1132_UCOEF3_REG, 0x00);

    uint8_t param_res = 0;

    // turn on UV Index, ALS IR, and ALS Visible
    param_res = setParameter(SI1132_PARAM_CHLIST, 0xB0);
   
    writeRegister(SI1132_ADDRESS, SI1132_HW_KEY_REG, 0x17);
    delay(10);
     
    // set up VIS sensor
    //  clock divide = 1
    param_res = setParameter(SI1132_PARAM_ALS_VIS_ADC_GAIN, 0x00);
    //  ADC count at 511
    param_res = setParameter(SI1132_PARAM_ALS_VIS_ADC_COUNTER, 0b01110000);
    //  set for high sensitivity
    param_res = setParameter(SI1132_PARAM_ALS_VIS_ADC_MISC, 0b00100000);
 
    // set up IR sensor
    //  clock divide = 1
    setParameter(SI1132_PARAM_ALS_IR_ADC_GAIN, 0x00);
   //  ADS count at 511
    setParameter(SI1132_PARAM_ALS_IR_ADC_COUNTER, 0b01110000);
    //  small IR photodiode
    setParameter(SI1132_PARAM_ALS_IR_ADCMUX, 0x00);
    //  set IR_RANGE bit for high sensitivity
    param_res = setParameter(SI1132_PARAM_ALS_IR_ADC_MISC, 0b00100000);

    // Place ins forced measurement mode
    writeRegister(SI1132_ADDRESS, _MEAS_RATE0, 0x00);
    writeRegister(SI1132_ADDRESS, _MEAS_RATE1, 0x00);

}

#define LOOP_TIMEOUT_MS 200
bool Si1132Sensor::waitUntilSleep(void) {

    for (int16_t i = 0; i < LOOP_TIMEOUT_MS; i++ ) {
        uint8_t val = readRegister(SI1132_ADDRESS, SI1132_CHIP_STAT_REG);
        if (val == 0b00000001) {
            // chip is asleep
            return true;
        }
        delay(1);
    }
    return false;
}

uint8_t Si1132Sensor::readResponseRegister(void)
{
    return readRegister(SI1132_ADDRESS, SI1132_RESPONSE_REG);
}

bool Si1132Sensor::sendCommand(uint8_t cmd_value)
{
    // RANT
    // If you were to say "hey, this is not what the Si1132 documentations says is the proper
    // command sending sequence", I would say "I tried that, It doesn't work. All sample code 
    // does this. Go ahead and follow the documentation if you like. You'll be wasting 2 days
    // of you life."
    // /RANT
    
    // write command value
    return writeRegister(SI1132_ADDRESS, SI1132_COMMAND_REG, cmd_value);
}

uint8_t Si1132Sensor::setParameter(uint8_t param, uint8_t value)
{
    uint8_t cmd = SI1132_CMD_PARAM_SET | param;
    // set PARAM_WR register
    writeRegister(SI1132_ADDRESS, SI1132_PARAM_WR_REG, value);
    // send command
    bool success = sendCommand(cmd);
   
    if (!success) {
        // read parameter
        sendCommand(SI1132_CMD_PARAM_QUERY|param);
        uint8_t paramValue = readRegister(SI1132_ADDRESS, SI1132_PARAM_RD_REG);
        Serial.print(F("ERROR writing parameter = 0x"));
        Serial.print(param, HEX);
        Serial.print(F(", final parameter value = "));
        Serial.print(paramValue, HEX);
        Serial.print(F(", desired parameter value = "));
        Serial.print(value, HEX);
        Serial.print(F("\n"));
        return 0;
    }
    // read results in PARAM_RD
    return readRegister(SI1132_ADDRESS, SI1132_PARAM_RD_REG);
}

const uint8_t* Si1132Sensor::getCurrentMeasurementBuffer(void)
{
    // start the measurements
    sendCommand(SI1132_CMD_ALS_FORCE);

    // wait for measurements
    delay(1000);
    // UV index
    _buffer[0] = readRegister(SI1132_ADDRESS, SI1132_AUX_DATA1_REG);
    _buffer[1] = readRegister(SI1132_ADDRESS, SI1132_AUX_DATA0_REG);
    _buffer[2] = readRegister(SI1132_ADDRESS, SI1132_ALS_VIS_DATA1_REG);
    _buffer[3] = readRegister(SI1132_ADDRESS, SI1132_ALS_VIS_DATA0_REG);
    _buffer[4] = readRegister(SI1132_ADDRESS, SI1132_ALS_IR_DATA1_REG);
    _buffer[5] = readRegister(SI1132_ADDRESS, SI1132_ALS_IR_DATA0_REG);

    Serial.print(F("UV index = "));
    Serial.print(_buffer[0]*256+_buffer[1]);
    Serial.print(F(", visible = "));
    Serial.print(_buffer[2]*256+_buffer[3]);
    Serial.print(F(", IR = "));
    Serial.print(_buffer[4]*256+_buffer[5]);
    Serial.print(F("\n"));

    return _buffer;
}
