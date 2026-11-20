// PCM5122.cpp
// Created on: 14.11.2026
// Updated on:

#include "PCM5122.h"

PCM5122::PCM5122() {}

PCM5122::~PCM5122() {}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool PCM5122::begin(TwoWire* twi, uint8_t address) {

    if (!twi) return false;

    m_addr = address;
    m_TwoWireInstance = twi;

    m_TwoWireInstance->beginTransmission(m_addr);

    if (m_TwoWireInstance->endTransmission() != 0) {
        printf(ANSI_ESC_RED "[%s:%i] PCM5122 not found\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        return false;
    }

    // I2S, 32 Bit
    if (!WriteReg(REG_I2S_CONFIG, I2S_32BIT)) return false;

    printf(ANSI_ESC_GREEN "[%s:%i] CM5122 initialized\n" ANSI_ESC_RESET, __FILE__, __LINE__);

    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool PCM5122::setVolume(uint8_t volume) {
    volume = constrain(volume, 0, 100);

    uint8_t regValue;

    if (volume == 0) {
        regValue = 0xFF;
    } else {
        uint8_t attenuation = ((100 - volume) * 120 + 49) / 99;
        regValue = 0x30 + attenuation;
    }

    if (!WriteReg(0x3D, regValue)) return false;
    if (!WriteReg(0x3E, regValue)) return false;

    m_volume = volume;
    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool PCM5122::setSampleRate(uint32_t sampleRate) {

    if (sampleRate == m_sampleRate) return true;

    if (!configurePLL(sampleRate)) return false;

    if (!waitForPllLock((uint32_t)100)) return false;

    m_sampleRate = sampleRate;
    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool PCM5122::WriteReg(uint8_t reg, uint8_t val) {
    if (!m_TwoWireInstance) return false;

    // Page 0 auswählen
    m_TwoWireInstance->beginTransmission(m_addr);
    m_TwoWireInstance->write(REG_PAGE_SELECT);
    m_TwoWireInstance->write(0x00);

    if (m_TwoWireInstance->endTransmission() != 0) return false;

    // Register schreiben
    m_TwoWireInstance->beginTransmission(m_addr);
    m_TwoWireInstance->write(reg);
    m_TwoWireInstance->write(val);

    return (m_TwoWireInstance->endTransmission() == 0);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool PCM5122::ReadReg(uint8_t reg, uint8_t& val) {
    if (!m_TwoWireInstance) return false;

    // Page 0 auswählen
    m_TwoWireInstance->beginTransmission(m_addr);
    m_TwoWireInstance->write(REG_PAGE_SELECT);
    m_TwoWireInstance->write(0x00);

    if (m_TwoWireInstance->endTransmission() != 0) return false;

    // Registeradresse setzen
    m_TwoWireInstance->beginTransmission(m_addr);
    m_TwoWireInstance->write(reg);

    if (m_TwoWireInstance->endTransmission(false) != 0) return false;

    if (m_TwoWireInstance->requestFrom(m_addr, (uint8_t)1) != 1) return false;

    val = m_TwoWireInstance->read();
    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void PCM5122::dumpRegisters() {

    static constexpr uint8_t regs[] = {
        0x25, // Clock configuration
        0x5E, // Clock status
        0x5F, // Clock error status
        0x76, // Power state
        0x6C  // Analog mute monitor
    };

    uint8_t val;

    // Page 0 auswählen
    if (!WriteReg(0x00, 0x00)) {
        printf(ANSI_ESC_RED "[%s:%i] PCM5122: Cannot select register page\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        return;
    }

    for (uint8_t reg : regs) {
        if (ReadReg(reg, val)) {
            printf(ANSI_ESC_GREEN "[%s:%i] PCM5122: Register 0x%02X = 0x%02X\n" ANSI_ESC_RESET, __FILE__, __LINE__, reg, val);
        } else {
            printf(ANSI_ESC_RED "[%s:%i] PCM5122: Read error at register 0x%02X\n" ANSI_ESC_RESET, __FILE__, __LINE__, reg);
        }
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool PCM5122::calculatePLL(uint32_t sampleRate, PLLConfig& cfg) {

    struct RatePreset {
        uint32_t sampleRate;
        uint32_t pllFrequency;
        uint8_t  p;
        uint8_t  j;
        uint8_t  r;
        uint8_t  dspDiv;
        uint8_t  dacDiv;
        uint8_t  ncpDiv;
        uint8_t  osrDiv;
        uint8_t  fsSpeed;
    };

    static constexpr RatePreset presets[] = {

        // SampleRate, PLL, P, J, R, DSP, DAC, NCP, OSR, FS

        {16000, 98304000, 1, 48, 2, 6, 16, 4, 24, 0}, //
        {22050, 90316800, 1, 32, 2, 4, 16, 4, 16, 0}, //
        {32000, 98304000, 1, 24, 2, 3, 16, 4, 12, 0}, //
        {44100, 90316800, 1, 16, 2, 2, 16, 4, 8, 0},  //
        {48000, 98304000, 1, 32, 1, 2, 16, 4, 8, 0},  //
        {88200, 90316800, 1, 16, 1, 2, 16, 4, 4, 1},  //
        {96000, 98304000, 1, 16, 1, 2, 16, 4, 4, 1},  //
    };

    for (const auto& preset : presets) {

        if (preset.sampleRate != sampleRate) continue;

        uint64_t bclk = uint64_t(sampleRate) * 64;

        // PLL input range: 1 MHz ... 50 MHz
        if (bclk < 1000000 || bclk > 50000000) return false;

        uint64_t pll = (bclk * preset.r * preset.j) / preset.p;

        if (pll != preset.pllFrequency) return false;

        cfg.p = preset.p;
        cfg.j = preset.j;
        cfg.d = 0;
        cfg.r = preset.r;

        cfg.dspDiv = preset.dspDiv;
        cfg.dacDiv = preset.dacDiv;
        cfg.ncpDiv = preset.ncpDiv;
        cfg.osrDiv = preset.osrDiv;

        cfg.fsSpeed = preset.fsSpeed;

        cfg.pllFrequency = preset.pllFrequency;

        // printf(ANSI_ESC_GREEN "[%s:%i] PCM5122 PLL: Fs=%lu BCLK=%llu PLL=%lu\n" ANSI_ESC_RESET, __FILE__, __LINE__, sampleRate, bclk, cfg.pllFrequency);
        // printf(ANSI_ESC_GREEN "[%s:%i] PCM5122 PLL: P=%u J=%u D=%u R=%u\n" ANSI_ESC_RESET, __FILE__, __LINE__, cfg.p, cfg.j, cfg.d, cfg.r);

        return true;
    }

    printf(ANSI_ESC_RED "[%s:%i] PCM5122: Unsupported sample rate %lu\n" ANSI_ESC_RESET, __FILE__, __LINE__, sampleRate);
    return false;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool PCM5122::configurePLL(uint32_t sampleRate) {

    PLLConfig cfg{};

    // Calculate PLL Configuration
    if (!calculatePLL(sampleRate, cfg)) return false;

    // Check Parameters
    if (cfg.p < 1 || cfg.p > 15 || cfg.j < 4 || cfg.j > 63 || cfg.d > 9999 || cfg.r < 1 || cfg.r > 16) {
        printf(ANSI_ESC_RED "[%s:%i] PCM5122: Invalid PLL parameters\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        return false;
    }

    if (cfg.dspDiv < 1 || cfg.dspDiv > 128 || cfg.dacDiv < 1 || cfg.dacDiv > 128 || cfg.ncpDiv < 1 || cfg.ncpDiv > 128 || cfg.osrDiv < 1 || cfg.osrDiv > 128) {
        printf(ANSI_ESC_RED "[%s:%i] PCM5122: Invalid clock divider\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        return false;
    }

    // -------------------------------------------------
    // 1. Disable Automatic Clock
    //    DCAS = Bit 1
    //    IDSK = Bit 4 (Ignore SCK-Error Detection)
    // -------------------------------------------------

    uint8_t clockCfg;

    if (!ReadReg(REG_CLOCK_CFG, clockCfg)) return false;

    clockCfg |= 0x1A;

    if (!WriteReg(REG_CLOCK_CFG, clockCfg)) return false;

    // -------------------------------------------------
    // 2. Disable PLL
    // -------------------------------------------------

    if (!WriteReg(REG_PLL_CTRL, 0x00)) return false;

    // -------------------------------------------------
    // 3. Select BCLK as the PLL reference
    //    SREF = 001
    // -------------------------------------------------

    if (!WriteReg(REG_PLL_REF, 0x10)) return false;

    // -------------------------------------------------
    // 4. Programming PLL Parameters
    // -------------------------------------------------

    if (!WriteReg(REG_PLL_P, cfg.p - 1)) return false;

    if (!WriteReg(REG_PLL_J, cfg.j)) return false;

    if (!WriteReg(REG_PLL_D_MSB, (cfg.d >> 8) & 0x3F)) return false;

    if (!WriteReg(REG_PLL_D_LSB, cfg.d & 0xFF)) return false;

    if (!WriteReg(REG_PLL_R, cfg.r - 1)) return false;

    // -------------------------------------------------
    // 5. Program the clock divider
    // Register value = divider - 1
    // -------------------------------------------------

    if (!WriteReg(REG_DSP_DIV, cfg.dspDiv - 1)) return false;

    if (!WriteReg(REG_DAC_DIV, cfg.dacDiv - 1)) return false;

    if (!WriteReg(REG_NCP_DIV, cfg.ncpDiv - 1)) return false;

    if (!WriteReg(REG_OSR_DIV, cfg.osrDiv - 1)) return false;

    // -------------------------------------------------
    // 6. Set FS Speed Mode
    //    Bit 1:0 = FSSP
    // -------------------------------------------------

    uint8_t fsSpeed;

    if (!ReadReg(REG_FS_SPEED, fsSpeed)) return false;

    fsSpeed = (fsSpeed & ~0x03) | (cfg.fsSpeed & 0x03);

    if (!WriteReg(REG_FS_SPEED, fsSpeed)) return false;

    // -------------------------------------------------
    // 7. Configure IDAC
    // DSP clock cycles per audio frame
    // -------------------------------------------------

    uint16_t idac = 0;

    switch (cfg.fsSpeed) {
        case 0: idac = 1024; break; // Single Speed
        case 1: idac = 512; break;  // Double Speed
        case 2: idac = 256; break;  // Quad Speed
        case 3: idac = 128; break;  // Octal Speed
        default: return false;
    }

    if (!WriteReg(REG_IDAC_MSB, idac >> 8)) return false;

    if (!WriteReg(REG_IDAC_LSB, idac & 0xFF)) return false;

    // -------------------------------------------------
    // 8. activate PLL
    // -------------------------------------------------

    if (!WriteReg(REG_PLL_CTRL, 0x01)) return false;

    printf(ANSI_ESC_GREEN "[%s:%i] PCM5122 PLL configured for %lu Hz\n" ANSI_ESC_RESET, __FILE__, __LINE__, sampleRate);

    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool PCM5122::isPllLocked() {

    uint8_t val;

    if (!ReadReg(REG_PLL_CTRL, val)) return false;

    return ((val & 0x10) == 0) && ((val & 0x01) != 0);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool PCM5122::waitForPllLock(uint32_t timeoutMs) {

    uint32_t start = millis();
    uint8_t  val;

    while ((millis() - start) < timeoutMs) {

        if (!ReadReg(REG_PLL_CTRL, val)) {
            printf(ANSI_ESC_RED "[%s:%i] PCM5122: PLL status read error\n" ANSI_ESC_RESET, __FILE__, __LINE__);
            return false;
        }

        // PLL must be enabled and locked in
        if ((val & 0x11) == 0x01) {
            printf(ANSI_ESC_RED "[%s:%i] PCM5122: PLL locked (%lu ms)\n" ANSI_ESC_RESET, __FILE__, __LINE__, millis() - start);
            return true;
        }

        delay(1);
    }

    printf(ANSI_ESC_RED "[%s:%i] PCM5122: PLL lock timeout (%lu ms)\n" ANSI_ESC_RESET, __FILE__, __LINE__, timeoutMs);

    return false;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
