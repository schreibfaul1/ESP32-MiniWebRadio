// first release on 01/2025
// updated on Feb 02 2025


#include <Arduino.h>
#include <Wire.h>

extern __attribute__((weak)) void tp_positionXY(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_moved(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_pressed(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_long_pressed(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_released(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_long_released(uint16_t x, uint16_t y);

class TP_GT911{
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
    enum Rotate { _0, _90, _180, _270 };
    enum : uint8_t { GT911_MODE_INTERRUPT, GT911_MODE_POLLING };
    enum : uint8_t { GT911};

  public:
    TP_GT911(TwoWire* twi);
    bool     begin(int8_t sda, int8_t scl, uint8_t addr, uint32_t clkint8_t = 400000, int8_t intPin = -1, int8_t rstPin = -1);
    void     loop();
    void     setVersion(uint8_t v);
    void     setRotation(uint8_t m);
    void     setMirror(bool h, bool v);
    bool     getProductID();
    uint8_t  touched(uint8_t mode);
    GTPoint  getPoint(uint8_t num);
    GTPoint* getPoints();
    bool     readTouchPoints();

  private:
    TwoWire* m_wire;
    int8_t   m_scl;
    int8_t   m_sda;
    int8_t   m_addr;
    uint32_t m_clk;
    int8_t   m_intPin;
    int8_t   m_rstPin;
    uint8_t  m_rotation = 0;
    uint8_t  m_version = GT911;
    bool     m_mirror_h = false;
    bool     m_mirror_v = false;
    bool     m_f_isTouch = false;
    bool     m_f_isLongPressed = false;

    uint32_t m_pressingTime = 0;
    bool     m_f_isPressing = false;
    bool     m_f_longPressed = false;

  private:
    bool     readBytes(uint16_t reg, uint8_t* data, uint16_t size);
    bool     writeBytes(uint16_t reg, uint8_t* data, uint16_t size);
    void     reset();
    bool     write(uint16_t reg, uint8_t data);
    uint8_t  read(uint16_t reg);
    uint8_t  calcChecksum(uint8_t* buf, uint8_t len);
    uint8_t  readChecksum();
    int8_t   readTouches();
    GTInfo*  readInfo();
};
