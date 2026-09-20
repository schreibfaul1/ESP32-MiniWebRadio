#pragma once
#include <Arduino.h>
#include <Wire.h>

#define TCA9554_REG_INPUT_PORT       0x00
#define TCA9554_REG_OUTPUT_PORT      0x01
#define TCA9554_REG_POLARITY_INVERT  0x02
#define TCA9554_REG_CONFIG           0x03
#define TCA9554_OK                   0x00
#define TCA9554_PIN_ERROR            0x81
#define TCA9554_I2C_ERROR            0x82
#define TCA9554_VALUE_ERROR          0x83
#define TCA9554_PORT_ERROR           0x84
#define TCA9554_INVALID_READ         -100
#define TCA9554_INPUT_PORT_REGISTER  0x00 //  read()
#define TCA9554_OUTPUT_PORT_REGISTER 0x01 //  write()
#define TCA9554_POLARITY_REGISTER    0x02 //  get/setPolarity()
#define TCA9554_CONFIGURATION_PORT   0x03 //  pinMode()

enum class TCA_pinMode : uint8_t {Input, Output};

class TCA9554 {
  public:
    TCA9554();

    bool    begin(TwoWire* myWire, uint8_t address);
    bool    isConnected();
    uint8_t getAddress();
    bool    pinMode(uint8_t pin, TCA_pinMode mode);
    bool    write(uint8_t pin, uint8_t value);
    uint8_t read(uint8_t pin);
    bool    setPolarity(uint8_t pin, uint8_t value); //  input pins only.
    uint8_t getPolarity(uint8_t pin);
    int     lastError();

  private:
    bool    writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);

    uint8_t  m_address;
    TwoWire* m_wire;
    uint8_t  m_error;
};
