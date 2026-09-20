#include "TCA9554.h"

TCA9554::TCA9554() {
    m_error = TCA9554_OK;
}

bool TCA9554::begin(TwoWire* myWire, uint8_t address) {
    m_wire = myWire;
    m_address = address; // Store one of the two available addresses

    m_wire->beginTransmission(m_address);
    if(m_wire->endTransmission() != 0) return false;
    return true;
}

bool TCA9554::isConnected() {
    m_wire->beginTransmission(m_address);
    return (m_wire->endTransmission() == 0);
}

uint8_t TCA9554::getAddress() {
    return m_address;
}

bool TCA9554::pinMode(uint8_t pin, TCA_pinMode mode) {
    if (pin > 7) {
        m_error = TCA9554_PIN_ERROR;
        return false;
    }
    uint8_t val = readRegister(TCA9554_CONFIGURATION_PORT);
    uint8_t prevVal = val;
    uint8_t mask = 1 << pin;
    if (mode == TCA_pinMode::Input) {
        val |= mask;
    } else {
        val &= ~mask;
    }
    if (val != prevVal) { return writeRegister(TCA9554_CONFIGURATION_PORT, val); }
    m_error = TCA9554_OK;
    return true;
}

bool TCA9554::write(uint8_t pin, uint8_t value) {
    if (pin > 7) {
        m_error = TCA9554_PIN_ERROR;
        return false;
    }
    uint8_t val = readRegister(TCA9554_OUTPUT_PORT_REGISTER);
    uint8_t prevVal = val;
    uint8_t mask = 1 << pin;
    if (value)
        val |= mask; //  all values are HIGH.
    else
        val &= ~mask;
    if (val != prevVal) { return writeRegister(TCA9554_OUTPUT_PORT_REGISTER, val); }
    m_error = TCA9554_OK;
    return true;
}

uint8_t TCA9554::read(uint8_t pin) {
    if (pin > 7) {
        m_error = TCA9554_PIN_ERROR;
        return TCA9554_INVALID_READ;
    }
    uint8_t val = readRegister(TCA9554_INPUT_PORT_REGISTER);
    uint8_t mask = 1 << pin;
    m_error = TCA9554_OK;
    if (val & mask) return HIGH;
    return LOW;
}

bool TCA9554::setPolarity(uint8_t pin, uint8_t value) {
    if (pin > 7) {
        m_error = TCA9554_PIN_ERROR;
        return false;
    }
    if ((value != LOW) && (value != HIGH)) {
        m_error = TCA9554_VALUE_ERROR;
        return false;
    }
    uint8_t val = readRegister(TCA9554_POLARITY_REGISTER);
    uint8_t prevVal = val;
    uint8_t mask = 1 << pin;
    if (value == HIGH)
        val |= mask;
    else
        val &= ~mask;
    if (val != prevVal) { return writeRegister(TCA9554_POLARITY_REGISTER, val); }
    m_error = TCA9554_OK;
    return true;
}

uint8_t TCA9554::getPolarity(uint8_t pin) {
    if (pin > 7) {
        m_error = TCA9554_PIN_ERROR;
        return false;
    }
    m_error = TCA9554_OK;
    uint8_t mask = readRegister(TCA9554_POLARITY_REGISTER);
    return (mask >> pin) == 0x01;
}

int TCA9554::lastError() {
    int error = m_error;
    m_error = TCA9554_OK; //  reset error after read.
    return error;
}

bool TCA9554::writeRegister(uint8_t reg, uint8_t value) {
    m_wire->beginTransmission(m_address);
    m_wire->write(reg);
    m_wire->write(value);
    if (m_wire->endTransmission() != 0) {
        m_error = TCA9554_I2C_ERROR;
        return false;
    }
    m_error = TCA9554_OK;
    return true;
}

uint8_t TCA9554::readRegister(uint8_t reg) {
    m_wire->beginTransmission(m_address);
    m_wire->write(reg);
    int rv = m_wire->endTransmission();
    if (rv != 0) {
        m_error = TCA9554_I2C_ERROR;
        return rv;
    } else {
        m_error = TCA9554_OK;
    }
    m_wire->requestFrom(m_address, (uint8_t)1);
    return m_wire->read();
}
