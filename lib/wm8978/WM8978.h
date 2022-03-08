#ifndef __WM8978_H
#define __WM8978_H

#include <stdio.h>

#define WM8978_ADDR   0X1A  //WM8978��������ַ,�̶�Ϊ0X1A

#define EQ1_80Hz      0X00
#define EQ1_105Hz     0X01
#define EQ1_135Hz     0X02
#define EQ1_175Hz     0X03

#define EQ2_230Hz     0X00
#define EQ2_300Hz     0X01
#define EQ2_385Hz     0X02
#define EQ2_500Hz     0X03

#define EQ3_650Hz     0X00
#define EQ3_850Hz     0X01
#define EQ3_1100Hz    0X02
#define EQ3_14000Hz   0X03

#define EQ4_1800Hz    0X00
#define EQ4_2400Hz    0X01
#define EQ4_3200Hz    0X02
#define EQ4_4100Hz    0X03

#define EQ5_5300Hz    0X00
#define EQ5_6900Hz    0X01
#define EQ5_9000Hz    0X02
#define EQ5_11700Hz   0X03

class WM8978
{
  public:
    WM8978() {}
    ~WM8978() {}
    bool begin(); /* use this function if you want to setup i2c before */
    bool begin(const int8_t sda, const int8_t scl, const uint32_t frequency = 100000);
    void cfgADDA(uint8_t dacen, uint8_t adcen);
    void cfgInput(uint8_t micen, uint8_t lineinen, uint8_t auxen);
    void cfgOutput(uint8_t dacen, uint8_t bpsen);
    void cfgI2S(uint8_t fmt, uint8_t len);
    void setMICgain(uint8_t gain);
    void setLINEINgain(uint8_t gain);
    void setAUXgain(uint8_t gain);
    void SetVolumeHeadphone(uint8_t volx);
    void SetVolumeSpeaker(uint8_t volx);
    void set3D(uint8_t depth);
    void set3Ddir(uint8_t dir);
    void setEQ1(uint8_t cfreq, uint8_t gain);
    void setEQ2(uint8_t cfreq, uint8_t gain);
    void setEQ3(uint8_t cfreq, uint8_t gain);
    void setEQ4(uint8_t cfreq, uint8_t gain);
    void setEQ5(uint8_t cfreq, uint8_t gain);
    void setNoise(uint8_t enable, uint8_t gain);
    void setALC(uint8_t enable, uint8_t maxgain, uint8_t mingain);
    void setHPF(uint8_t enable);

  private:
    uint8_t Init(void);
    uint8_t Write_Reg(uint8_t reg, uint16_t val);
    uint16_t Read_Reg(uint8_t reg);
};
#endif
