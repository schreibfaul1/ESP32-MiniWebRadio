#include "BH1750.h"

//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
hp_BH1750::hp_BH1750(TwoWire *myWire){  // constructor
    _wire = myWire;
	m_sensitivity = SENSITIVITY_ADJ_DEFAULT;
	m_measurementTime = 120;
	m_resolution = 1.0;
}
hp_BH1750::~hp_BH1750(){  // destructor
	_wire->end();
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool hp_BH1750::setSensitivity(uint8_t sensitivity){
	if(sensitivity < SENSITIVITY_ADJ_MIN) return false;
	if(sensitivity > SENSITIVITY_ADJ_MAX) return false;
	m_sensitivity = sensitivity;
	return writeMtreg(m_sensitivity); // Set standard sensitivity
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool hp_BH1750::setResolutionMode(uint8_t resolutionMode){
	if(resolutionMode == ONE_TIME_H_RESOLUTION_MODE) { m_resolution = 1.0;  m_measurementTime = 120; m_resolutionMode = resolutionMode;  return true;}
	if(resolutionMode == ONE_TIME_H_RESOLUTION_MODE2){ m_resolution = 0.5;  m_measurementTime = 120; m_resolutionMode = resolutionMode;  return true;}
	if(resolutionMode == ONE_TIME_L_RESOLUTION_MODE) { m_resolution = 4.0;  m_measurementTime = 16;  m_resolutionMode = resolutionMode;  return true;}
	return false;
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool hp_BH1750::begin(uint8_t address, int8_t sda, int8_t scl) {
    _address = address;             // Store one of the two available addresses
    _wire->begin(sda, scl, 400000); // Initialisation of wire object with standard SDA/SCL lines
    return writeMtreg(BH1750_MTREG_DEFAULT); // Set standard sensitivity
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool hp_BH1750::powerOn(){
    return writeByte(0x1);
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// For low power consuming. This mode is entered automatically if a measurement is finished
bool hp_BH1750::powerOff() {
	return writeByte(0x0);
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool hp_BH1750::reset() {
    if(powerOn() == false) return false;
    return writeByte(0x7);
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool hp_BH1750::writeByte(uint8_t b){  // Sends command to sensor
    _wire->beginTransmission(_address);
    _wire->write(b);
    return (_wire->endTransmission() == 0);
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int32_t hp_BH1750::readValue() {
    uint8_t  buff[2];
    uint16_t req = _wire->requestFrom((int)_address, (int)2); // request two bytes
    if(req < 2 || _wire->available() < 2) {
        return -1; // Sensor not ready
    }
    buff[0] = _wire->read(); // Receive one byte
    buff[1] = _wire->read(); // Receive one byte
    return ((buff[0] << 8) | buff[1]);
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool hp_BH1750::writeMtreg(uint8_t mtreg) { // Change sensitivity measurement time
    uint8_t hiByte = mtreg >> 5;
    hiByte |= 0b01000000;// High bit: 01000_MT[7,6,5]
    writeByte(hiByte);
    uint8_t loByte = mtreg & 0b00011111;
    loByte |= 0b01100000;// Low bit:  011_MT[4,3,2,1,0]
    return writeByte(loByte);
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool hp_BH1750::start(){ // Start a single shot measurement with given resolution and sensitivity
  reset(); // Reset the last result in data register to zero (0)
  return writeByte(m_resolutionMode);
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t hp_BH1750::getBrightness(){
  return (float)readValue() / ((float)SENSITIVITY_ADJ_DEFAULT / m_sensitivity) * m_resolution;;
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
