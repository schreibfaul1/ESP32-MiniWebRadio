// light sensor BH1750
#include <Arduino.h>

#include <Wire.h>

#pragma once

extern __attribute__((weak)) void on_BH1750(int32_t);

static const unsigned int BH1750_SATURATED = 65535;
enum BH1750Quality {
    BH1750_QUALITY_HIGH = 0x20,
    BH1750_QUALITY_HIGH2 = 0x21,
    BH1750_QUALITY_LOW = 0x23,
};
enum BH1750CalResult {
    BH1750_CAL_OK = 0,
    BH1750_CAL_MTREG_CHANGED = 1,
    BH1750_CAL_TOO_BRIGHT = 2,
    BH1750_CAL_TOO_DARK = 3,
    BH1750_CAL_COMMUNICATION_ERROR = 4,
};
struct BH1750Timing {
    uint8_t      mtregLow;
    uint8_t      mtregHigh;
    unsigned int mtregLow_qualityHigh;
    unsigned int mtregHigh_qualityHigh;
    unsigned int mtregLow_qualityLow;
    unsigned int mtregHigh_qualityLow;
};

enum BH1750MtregLimit {
    BH1750_MTREG_LOW = 31, // the datashet specifies 31 as minimum value
    // but you can go even lower (depending on your specific chip)
    // BH1750_MTREG_LOW=5 works with my chip and enhances the range
    // from 121.556,8 Lux to 753.652,5 Lux.
    BH1750_MTREG_HIGH = 254,
    BH1750_MTREG_DEFAULT = 69
};

enum BH1750Address { BH1750_TO_GROUND = 0x23, BH1750_TO_VCC = 0x5C };
extern TwoWire Wire; /**< Forward declaration of Wire object */

class hp_BH1750 {

public:
    hp_BH1750();
	~hp_BH1750();

public:
	const uint8_t ONE_TIME_H_RESOLUTION_MODE      = 0b00100000; // Start measurement at 1lx resolution. Measurement Time is typically 120ms. Power Down mode after measurement.
	const uint8_t ONE_TIME_H_RESOLUTION_MODE2     = 0b00100001; // Start measurement at 0.5lx resolution. Measurement Time is typically 120ms. Power Down mode after measurement.
	const uint8_t ONE_TIME_L_RESOLUTION_MODE      = 0b00100011; // Start measurement at 4lx resolution. Measurement Time is typically 16ms. Power Down mode after measurement.

	const uint8_t ADDR_TO_GROUND                  = 0x23;
	const uint8_t ADDR_TO_VCC                     = 0x5C;

	const uint8_t SENSITIVITY_ADJ_MIN             = 31;         // Adjust measurement result for influence of optical window. (sensor sensitivity adjusting)
	const uint8_t SENSITIVITY_ADJ_DEFAULT         = 69;
	const uint8_t SENSITIVITY_ADJ_MAX             = 254;




	bool         setSensitivity(uint8_t sensitivity);
    bool         setResolutionMode(uint8_t resolution);
    bool         begin(TwoWire* myWire, uint8_t address);
    bool         writeByte(uint8_t b);
    int32_t      readValue();
    bool         reset();
    bool         powerOn();
    bool         powerOff();
    bool         writeMtreg(uint8_t mtreg);
    bool         isInit = false;



    void         setQuality(BH1750Quality quality);
    bool         start();
    float        getLux();
    uint16_t     getBrightness();
    void         loop();


    unsigned int getMtregTime() const;
//    unsigned int getMtregTime(byte mtreg) const;
    unsigned int getMtregTime(uint8_t mtreg, BH1750Quality quality) const;



private:
    TwoWire*      _wire;
    uint8_t       _address;
    uint8_t       _mtreg;
    uint8_t       _percent = 50;
    uint32_t      _timer = 0;
    bool          _timeflag = false;
    bool          _processed = false;
    unsigned int  _mtregTime;
    unsigned long _startMillis;
    unsigned long _resultMillis;
    unsigned long _timeoutMillis;
    unsigned long _timeout = 10;
    int           _offset = 0;
    unsigned int  _nReads;
    unsigned int  _time;
    unsigned int  _value;
    float         _qualFak = 0.5;
    float         luxCache;
    BH1750Quality _quality;
    BH1750Timing  _timing;

    uint8_t checkMtreg(uint8_t mtreg);


   // unsigned int readValue();
    unsigned int readChange(uint8_t mtreg, BH1750Quality quality, bool change);

	uint8_t m_sensitivity     = SENSITIVITY_ADJ_DEFAULT;
	float   m_resolution      = 1.0;
	uint8_t m_resolutionMode  = ONE_TIME_H_RESOLUTION_MODE;
	uint8_t m_measurementTime = 120;

	const uint8_t m_Power_Down                    = 0b00000000; // No active state.
	const uint8_t m_Power_On                      = 0b00000001; // Waiting for measurement command.
	const uint8_t m_Reset                         = 0b00000111; // Reset Data register value. Reset command is not acceptable in Power Down mode.



};
