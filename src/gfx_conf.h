#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <driver/i2c.h>

/*******************************************************************************
 * Please define the corresponding macros based on the board you have purchased.
 * CrowPanel_43 means CrowPanel 4.3inch Board
 * CrowPanel_50 means CrowPanel 5.0inch Board
 * CrowPanel_70 means CrowPanel 7.0inch Board
 ******************************************************************************/
// #define CrowPanel_70
 #define CrowPanel_50
// #define CrowPanel_43


#if defined (CrowPanel_70)

#define screenWidth   800
#define screenHeight  480
class LGFX : public lgfx::LGFX_Device
{
public:
    lgfx::Bus_RGB _bus_instance;
    lgfx::Panel_RGB _panel_instance;
    lgfx::Light_PWM _light_instance;
    lgfx::Touch_GT911 _touch_instance;
    LGFX(void)
    {
        {
            auto cfg = _panel_instance.config();
            cfg.memory_width = screenWidth;
            cfg.memory_height = screenHeight;
            cfg.panel_width = screenWidth;
            cfg.panel_height = screenHeight;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            _panel_instance.config(cfg);
        }

        {
            auto cfg = _bus_instance.config();
            cfg.panel = &_panel_instance;

            cfg.pin_d0 = GPIO_NUM_15;  // B0
            cfg.pin_d1 = GPIO_NUM_7;  // B1
            cfg.pin_d2 = GPIO_NUM_6; // B2
            cfg.pin_d3 = GPIO_NUM_5;  // B3
            cfg.pin_d4 = GPIO_NUM_4;  // B4

            cfg.pin_d5 = GPIO_NUM_9;  // G0
            cfg.pin_d6 = GPIO_NUM_46;  // G1
            cfg.pin_d7 = GPIO_NUM_3;  // G2
            cfg.pin_d8 = GPIO_NUM_8; // G3
            cfg.pin_d9 = GPIO_NUM_16; // G4
            cfg.pin_d10 = GPIO_NUM_1; // G5

            cfg.pin_d11 = GPIO_NUM_14; // R0
            cfg.pin_d12 = GPIO_NUM_21; // R1
            cfg.pin_d13 = GPIO_NUM_47; // R2
            cfg.pin_d14 = GPIO_NUM_48; // R3
            cfg.pin_d15 = GPIO_NUM_45; // R4

            cfg.pin_henable = GPIO_NUM_41;
            cfg.pin_vsync = GPIO_NUM_40;
            cfg.pin_hsync = GPIO_NUM_39;
            cfg.pin_pclk = GPIO_NUM_0;
            cfg.freq_write = 12000000;

            cfg.hsync_polarity    = 0;
            cfg.hsync_front_porch = 40;
            cfg.hsync_pulse_width = 48;
            cfg.hsync_back_porch  = 40;
            
            cfg.vsync_polarity    = 0;
            cfg.vsync_front_porch = 1;
            cfg.vsync_pulse_width = 31;
            cfg.vsync_back_porch  = 13;

            cfg.pclk_active_neg = 1;
            cfg.de_idle_high = 0;
            cfg.pclk_idle_high = 0;

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = GPIO_NUM_2;
            _light_instance.config(cfg);
            _panel_instance.light(&_light_instance);
        }

        {
            auto cfg = _touch_instance.config();
            cfg.x_min      = 0;
            cfg.x_max      = 799;
            cfg.y_min      = 0;
            cfg.y_max      = 479;
            cfg.pin_int    = -1;
            cfg.pin_rst    = -1;
            cfg.bus_shared = true;
            cfg.offset_rotation = 0;
            cfg.i2c_port   = I2C_NUM_1;
            cfg.pin_sda    = GPIO_NUM_19;
            cfg.pin_scl    = GPIO_NUM_20;
            cfg.freq       = 400000;
            cfg.i2c_addr   = 0x14;
            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }
        setPanel(&_panel_instance);
    }
};

#elif defined (CrowPanel_50)

#define screenWidth   800
#define screenHeight  480
class LGFX : public lgfx::LGFX_Device
{
public:
    lgfx::Bus_RGB _bus_instance;
    lgfx::Panel_RGB _panel_instance;
    lgfx::Light_PWM _light_instance;
    lgfx::Touch_GT911 _touch_instance;
    LGFX(void)
    {
        {
            auto cfg = _panel_instance.config();
            cfg.memory_width = screenWidth;
            cfg.memory_height = screenHeight;
            cfg.panel_width = screenWidth;
            cfg.panel_height = screenHeight;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            _panel_instance.config(cfg);
        }

        {
            auto cfg = _bus_instance.config();
            cfg.panel = &_panel_instance;

            cfg.pin_d0 = GPIO_NUM_8;  // B0
            cfg.pin_d1 = GPIO_NUM_3;  // B1
            cfg.pin_d2 = GPIO_NUM_46; // B2
            cfg.pin_d3 = GPIO_NUM_9;  // B3
            cfg.pin_d4 = GPIO_NUM_1;  // B4

            cfg.pin_d5 = GPIO_NUM_5;  // G0
            cfg.pin_d6 = GPIO_NUM_6;  // G1
            cfg.pin_d7 = GPIO_NUM_7;  // G2
            cfg.pin_d8 = GPIO_NUM_15; // G3
            cfg.pin_d9 = GPIO_NUM_16; // G4
            cfg.pin_d10 = GPIO_NUM_4; // G5

            cfg.pin_d11 = GPIO_NUM_45; // R0
            cfg.pin_d12 = GPIO_NUM_48; // R1
            cfg.pin_d13 = GPIO_NUM_47; // R2
            cfg.pin_d14 = GPIO_NUM_21; // R3
            cfg.pin_d15 = GPIO_NUM_14; // R4

            cfg.pin_henable = GPIO_NUM_40;
            cfg.pin_vsync = GPIO_NUM_41;
            cfg.pin_hsync = GPIO_NUM_39;
            cfg.pin_pclk = GPIO_NUM_0;
            cfg.freq_write = 12000000;

            cfg.hsync_polarity    = 0;
            cfg.hsync_front_porch = 8;
            cfg.hsync_pulse_width = 4;
            cfg.hsync_back_porch  = 43;
            
            cfg.vsync_polarity    = 0;
            cfg.vsync_front_porch = 8;
            cfg.vsync_pulse_width = 4;
            cfg.vsync_back_porch  = 12;

            cfg.pclk_active_neg = 1;
            cfg.de_idle_high = 0;
            cfg.pclk_idle_high = 0;

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = GPIO_NUM_2;
            _light_instance.config(cfg);
            _panel_instance.light(&_light_instance);
        }

        {
            auto cfg = _touch_instance.config();
            cfg.x_min      = 0;
            cfg.x_max      = 799;
            cfg.y_min      = 0;
            cfg.y_max      = 479;
            cfg.pin_int    = -1;
            cfg.pin_rst    = -1;
            cfg.bus_shared = true;
            cfg.offset_rotation = 0;
            cfg.i2c_port   = I2C_NUM_1;
            cfg.pin_sda    = GPIO_NUM_19;
            cfg.pin_scl    = GPIO_NUM_20;
            cfg.freq       = 400000;
            cfg.i2c_addr   = 0x14;
            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }
        setPanel(&_panel_instance);
    }
};

#elif defined (CrowPanel_43)

#define screenWidth   480
#define screenHeight  272
class LGFX : public lgfx::LGFX_Device
{
public:
    lgfx::Bus_RGB _bus_instance;
    lgfx::Panel_RGB _panel_instance;
    lgfx::Light_PWM _light_instance;
    lgfx::Touch_XPT2046 _touch_instance;
    LGFX(void)
    {
        {
            auto cfg = _panel_instance.config();
            cfg.memory_width = screenWidth;
            cfg.memory_height = screenHeight;
            cfg.panel_width = screenWidth;
            cfg.panel_height = screenHeight;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            _panel_instance.config(cfg);
        }

        {
            auto cfg = _bus_instance.config();
            cfg.panel = &_panel_instance;

            cfg.pin_d0 = GPIO_NUM_8;  // B0
            cfg.pin_d1 = GPIO_NUM_3;  // B1
            cfg.pin_d2 = GPIO_NUM_46; // B2
            cfg.pin_d3 = GPIO_NUM_9;  // B3
            cfg.pin_d4 = GPIO_NUM_1;  // B4

            cfg.pin_d5 = GPIO_NUM_5;  // G0
            cfg.pin_d6 = GPIO_NUM_6;  // G1
            cfg.pin_d7 = GPIO_NUM_7;  // G2
            cfg.pin_d8 = GPIO_NUM_15; // G3
            cfg.pin_d9 = GPIO_NUM_16; // G4
            cfg.pin_d10 = GPIO_NUM_4; // G5

            cfg.pin_d11 = GPIO_NUM_45; // R0
            cfg.pin_d12 = GPIO_NUM_48; // R1
            cfg.pin_d13 = GPIO_NUM_47; // R2
            cfg.pin_d14 = GPIO_NUM_21; // R3
            cfg.pin_d15 = GPIO_NUM_14; // R4

            cfg.pin_henable = GPIO_NUM_40;
            cfg.pin_vsync = GPIO_NUM_41;
            cfg.pin_hsync = GPIO_NUM_39;
            cfg.pin_pclk = GPIO_NUM_42;
            cfg.freq_write = 8000000;

            cfg.hsync_polarity    = 0;
            cfg.hsync_front_porch = 8;
            cfg.hsync_pulse_width = 4;
            cfg.hsync_back_porch  = 43;
            
            cfg.vsync_polarity    = 0;
            cfg.vsync_front_porch = 8;
            cfg.vsync_pulse_width = 4;
            cfg.vsync_back_porch  = 12;

            cfg.pclk_active_neg = 1;
            cfg.de_idle_high = 0;
            cfg.pclk_idle_high = 0;

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = GPIO_NUM_2;
            _light_instance.config(cfg);
            _panel_instance.light(&_light_instance);
        }

        {
            auto touch_cfg = _touch_instance.config();
            touch_cfg.x_min      = 100;    // タッチスクリーンから得られる最小のX値(生の値)
            touch_cfg.x_max      = 4000;  // タッチスクリーンから得られる最大のX値(生の値)
            touch_cfg.y_min      = 100;    // タッチスクリーンから得られる最小のY値(生の値)
            touch_cfg.y_max      = 4000;  // タッチスクリーンから得られる最大のY値(生の値)
            touch_cfg.pin_int    = 36;   // INTが接続されているピン番号
            touch_cfg.bus_shared = true; // 画面と共通のバスを使用している場合 trueを設定
            touch_cfg.offset_rotation = 0;// 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定

            // SPI接続の場合
            touch_cfg.spi_host   = SPI2_HOST; //HSPI_HOST;// 使用するSPIを選択 (HSPI_HOST or VSPI_HOST)
            touch_cfg.freq       = 1000000;     // SPIクロックを設定
            touch_cfg.pin_sclk   = GPIO_NUM_12;     // SCLKが接続されているピン番号
            touch_cfg.pin_mosi   = GPIO_NUM_11;     // MOSIが接続されているピン番号
            touch_cfg.pin_miso   = GPIO_NUM_13;     // MISOが接続されているピン番号
            touch_cfg.pin_cs     = GPIO_NUM_0;     //   CSが接続されているピン番号

            _touch_instance.config(touch_cfg);
            _panel_instance.setTouch(&_touch_instance);  // タッチスクリーンをパネルにセットします。
        }
        setPanel(&_panel_instance);
    }
};
#endif

LGFX tft;