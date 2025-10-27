# MiniWebRadio Copilot Instructions

## Project Overview
ESP32-S3 internet radio with touchscreen UI, web interface, FTP server, and audio streaming (webradio, local files, DLNA). Built with PlatformIO using Arduino + ESP-IDF frameworks.

## Architecture

### Core Components
- **Main State Machine** (`src/main.cpp`): Central state management via `s_state` variable (RADIO, PLAYER, DLNA, CLOCK, etc.)
- **Audio Processing**: Runs on dedicated task (core 1) via `ESP32-audioI2S` library (external dependency)
- **Display Subsystem**: `lib/tftLib/` supports both SPI (ILI9341/ILI9486/ILI9488/ST7796) and RGB (800x480) displays
- **Web Server**: `lib/websrv/` handles HTTP (port 80) and WebSocket (port 81) for browser UI
- **DLNA Client**: `lib/dlna_client/` discovers and browses UPnP/DLNA media servers
- **FTP Server**: `lib/ftpsrv/` provides SD card access (default credentials: esp32/esp32)

### Configuration Pattern
All hardware config is centralized in `src/common.h`:
- GPIO pin assignments (different for SPI vs RGB displays)
- Display controller type via `TFT_CONTROLLER` define (0-7)
- WiFi credentials, FTP auth, timeouts, display frequencies
- I2S format (0=PCM5102A/MAX98357A, 1=LSBJ for PT8211)

### Multi-Core Architecture
- **Core 0**: Arduino `setup()`, `loop()`, UI, networking, FTP, web server
- **Core 1**: Audio task (decoding, I2S streaming) - set via `AUDIOTASK_CORE` in `platformio.ini`
- Loop task stack: 12KB (see `SET_LOOP_TASK_STACK_SIZE(12 * 1024)`)

### Data Persistence
- **SD_MMC**: Mandatory, stores station lists (`stations.csv`), audio files, station icons, settings
- **Preferences**: NVP storage for runtime settings (volume, tone, last state)
- **SPIFFS**: Embedded web UI assets (`index.h`, `index.js.h`)

## Development Workflow

### Building
```bash
# Select environment in platformio.ini (esp32s3 or esp32s3_OTA)
pio run -e esp32s3                    # Build for serial upload
pio run -e esp32s3 -t upload          # Upload via serial
pio run -e esp32s3_OTA -t upload      # OTA upload (requires 8MB+ flash)
```

### Environment Configuration
- Modify `platformio.ini` [esp32s3] section for board/partition selection
- Partition tables in `boards/miniwebradio{4,8,16,32}MB.csv`
- Custom boards defined in `boards/*.json`
- Pre-build script `env-extra.py` reads optional `.env` file for build flags

### Debugging
- Serial monitor at 921600 baud (set in `monitor_speed`)
- Colored ANSI output (extensive macros in `common.h`)
- Log level via `CORE_DEBUG_LEVEL` (0=None to 5=Verbose)
- SerialPrintfln() adds timestamps and buffers to web UI

## Code Conventions

### File Organization
- `src/main.cpp`: 5500+ lines, contains setup/loop and all state logic
- `src/mwr_src/*.hpp`: Split out functions (`function.hpp`), graphics (`graphical.hpp`), UI layout (`layout.hpp`)
- `lib/*/`: Self-contained components (each has .h/.cpp pair)

### String Handling
Use `ps_ptr<char>` smart pointer wrapper (project-specific) for dynamic strings instead of raw `char*` or `String`.

### State Transitions
Use `changeState(int32_t state)` function to switch modes. States defined in `enum status` (common.h). Always update `s_state` global variable.

### Display Updates
Call TFT drawing functions from main loop only (not from callbacks/interrupts). Use state-specific submenu variables: `s_radioSubMenue`, `s_playerSubMenue`, etc.

### Audio Callbacks
Handle events from audio library via functions like `audio_info()`, `audio_eof_mp3()`. These run on audio task core - don't do heavy processing.

### Web Interface
- HTTP requests trigger `WEBSRV_onCommand(cmd, param, arg)` callback
- WebSocket messages via `send(cmd, msg)` to browser
- JSON responses preferred for structured data

## Hardware Quirks

### SD Card
- Must use SD_MMC mode (not SPI) for stability/speed
- Remove pull-up on D0 for ESP32 (not needed for ESP32-S3)
- Some adapters have series resistors - remove and bridge
- Set frequency via `SDMMC_FREQUENCY` (80MHz recommended)

### Displays
- SPI displays: Set `TFT_CONTROLLER` (0,3,4,5), configure via `TFT_*` defines
- RGB displays: Set `TFT_CONTROLLER=7`, configure `RGB_PINS` and `RGB_TIMING` structs
- Touchpad: XPT2046 (SPI) or GT911 (I2C) - auto-selected based on controller

### I2S DACs
- PCM5102A/CS4344: Use `I2S_COMM_FMT=0`, check solder bridges on board
- PT8211: Requires `I2S_COMM_FMT=1` (LSBJ format)
- ES8388/AC101: Need I2C connection + I2S

## Common Patterns

### Adding a New State
1. Add enum value to `status` in `common.h`
2. Update `_hl_item[]` array in `main.cpp` with menu text
3. Add case to main loop state switch
4. Create submenu variable if needed (e.g., `s_newStateSubMenue`)
5. Add display rendering in `graphical.hpp`

### External Library Integration
Libraries like `Audio`, `IR`, `rtime` are external (from GitHub). Local wrappers/extensions go in `lib/*/`. See `platformio.ini` `lib_deps` section.

### Logging Best Practices
Use colored ANSI macros: `SerialPrintfln(ANSI_ESC_GREEN "Success" ANSI_ESC_RESET);`
Logs auto-buffer to web UI (see `s_logBuffer` deque).

## Key Files Reference
- `src/common.h`: All hardware config, includes, global declarations
- `platformio.ini`: Build config, board selection, Arduino/IDF versions
- `src/main.cpp`: Main application logic, state machine, setup/loop
- `src/mwr_src/graphical.hpp`: UI rendering functions
- `lib/websrv/`: Web/WebSocket server implementation
- `lib/dlna_client/`: DLNA/UPnP browsing and playback
- `boards/*.csv`: Partition tables for different flash sizes
