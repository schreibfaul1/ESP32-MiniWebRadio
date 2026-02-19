// first release on 01/2025
// updated on Nov 23 2025

#include <Arduino.h>
#include <Wire.h>

extern __attribute__((weak)) void tp_info(const char* info);
extern __attribute__((weak)) void tp_positionXY(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_moved(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_pressed(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_long_pressed(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_released(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_long_released(uint16_t x, uint16_t y);

class TP_GT911 {
  private:
    static const uint8_t  GT911_MAX_CONTACTS = 6;
    static const uint16_t GT911_REG_ID = 0x8140;
    static const uint16_t GT911_REG_DATA = 0x8140;
    static const uint16_t GT911_REG_COORD_ADDR = 0x814E;
    static const uint16_t GT911_REG_CHECKSUM = 0x80FF;
    static const uint16_t GT911_REG_CONFIG = 0x8047;

  public:
    struct __attribute__((packed)) GTPoint {
        uint8_t  trackId;
        uint16_t x;
        uint16_t y;
        uint16_t area;
        uint8_t  reserved;
    } m_points[GT911_MAX_CONTACTS];

    struct __attribute__((packed)) GTInfo {
        char     productId[4];
        uint16_t fwId;
        uint16_t xResolution;
        uint16_t yResolution;
        uint8_t  vendorId;
    } m_info;

  public:
    enum : uint8_t { GT911 };

  public:
    TP_GT911();
    bool     begin(TwoWire* twi, uint8_t addr, uint16_t h_resolution, uint16_t v_resolution);
    void     loop();
    void     setVersion(uint8_t v);
    void     setRotation(uint8_t m);
    void     setMirror(bool h, bool v);
    bool     getProductID();
    uint8_t  touched();
    GTPoint  getPoint(uint8_t num);
    GTPoint* getPoints();
    bool     readTouchPoints();

  private:
    TwoWire* m_wire;
    int8_t   m_scl;
    int8_t   m_sda;
    uint8_t  m_addr;
    uint32_t m_clk;
    int8_t   m_intPin;
    int8_t   m_rstPin;
    uint8_t  m_rotation = 0;
    uint8_t  m_version = GT911;
    uint16_t m_touch_h_resolution = 0;
    uint16_t m_touch_v_resolution = 0;
    uint16_t m_disp_h_resolution = 0;
    uint16_t m_disp_v_resolution = 0;
    bool     m_isInit = false;
    bool     m_mirror_h = false;
    bool     m_mirror_v = false;
    bool     m_f_isTouch = false;
    bool     m_f_isLongPressed = false;

    uint32_t m_pressingTime = 0;
    bool     m_f_isPressing = false;
    bool     m_f_longPressed = false;

  private:
    bool    readBytes(uint16_t reg, uint8_t* data, uint16_t size);
    bool    writeBytes(uint16_t reg, uint8_t* data, uint16_t size);
    void    reset();
    bool    write(uint16_t reg, uint8_t data);
    uint8_t read(uint16_t reg);
    uint8_t calcChecksum(uint8_t* buf, uint8_t len);
    uint8_t readChecksum();
    int8_t  readTouches();
    GTInfo* readInfo();
};
