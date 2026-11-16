#pragma once
#include <Arduino.h>
#include <Wire.h>

class PCM5122 {

  private:
    static constexpr uint8_t REG_PAGE_SELECT = 0x00;
    static constexpr uint8_t REG_I2S_CONFIG = 0x28;
    static constexpr uint8_t I2S_32BIT = 0x03;
    static constexpr uint8_t REG_PLL_CTRL = 0x04;
    static constexpr uint8_t REG_PLL_REF = 0x0D;

    static constexpr uint8_t REG_PLL_P = 0x14;
    static constexpr uint8_t REG_PLL_J = 0x15;
    static constexpr uint8_t REG_PLL_D_MSB = 0x16;
    static constexpr uint8_t REG_PLL_D_LSB = 0x17;
    static constexpr uint8_t REG_PLL_R = 0x18;

    static constexpr uint8_t REG_DSP_DIV = 0x1B;
    static constexpr uint8_t REG_DAC_DIV = 0x1C;
    static constexpr uint8_t REG_NCP_DIV = 0x1D;
    static constexpr uint8_t REG_OSR_DIV = 0x1E;

    static constexpr uint8_t REG_FS_SPEED = 0x22;
    static constexpr uint8_t REG_IDAC_MSB = 0x23;
    static constexpr uint8_t REG_IDAC_LSB = 0x24;
    static constexpr uint8_t REG_CLOCK_CFG = 0x25;

#define ANSI_ESC_RED   "\033[31m"
#define ANSI_ESC_GREEN "\033[32m"
#define ANSI_ESC_RESET "\033[0m"

    TwoWire* m_TwoWireInstance = NULL; // TwoWire Instance
    int8_t   m_addr = -1;
    uint32_t m_sampleRate = 0;
    uint8_t  m_volume = 0;

    struct PLLConfig {
        uint8_t  p;
        uint8_t  j;
        uint16_t d;
        uint8_t  r;

        uint8_t dspDiv;
        uint8_t dacDiv;
        uint8_t ncpDiv;
        uint8_t osrDiv;

        uint8_t fsSpeed;

        uint32_t pllFrequency;
    };

  public:
    PCM5122();
    ~PCM5122();
    bool    begin(TwoWire* twi, uint8_t address);
    bool    setSampleRate(uint32_t sample_rate);
    bool    setVolume(uint8_t volume);
    uint8_t getVolume() { return m_volume; }
    bool    isPllLocked();

  public:
    void dumpRegisters();

  protected:
    bool WriteReg(uint8_t reg, uint8_t val);
    bool ReadReg(uint8_t reg, uint8_t& val);

    bool calculatePLL(uint32_t sampleRate, PLLConfig& cfg);
    bool configurePLL(uint32_t sampleRate);
    bool waitForPllLock(uint32_t timeoutMs);
};
