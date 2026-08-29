#include "../common.h"

#pragma once

#ifdef TFT_MODE_DSI
TFT_DSI& getTFT();
#elif defined(TFT_MODE_SPI)
TFT_SPI& getTFT();
#elif defined(TFT_MODE_RGB)
TFT_RGB& getTFT();
#endif

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void make_hardcopy_on_sd() {
    const uint8_t bmp320x240[70] = {
        0x42, 0x4D, 0x46, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x58, 0x02, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    (void)bmp320x240;

    const uint8_t bmp480x320[70] = {
        0x42, 0x4D, 0x46, 0xB0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 0x40, 0x01,
        0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x04, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    (void)bmp480x320;

    const uint8_t bmp800x480[70] = {
        0x42, 0x4D, 0x46, 0xC4, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, // BM + File size 768070
        0x46, 0x00, 0x00, 0x00,                                     // Pixel data offset (70 bytes)
        0x28, 0x00, 0x00, 0x00,                                     // DIB header size
        0x20, 0x03, 0x00, 0x00,                                     // Width: 800
        0xE0, 0x01, 0x00, 0x00,                                     // Height: 480
        0x01, 0x00,                                                 // Color planes
        0x10, 0x00,                                                 // Bit count: 16 (RGB565)
        0x03, 0x00, 0x00, 0x00,                                     // Compression: BI_BITFIELDS (3)
        0x00, 0xB0, 0x04, 0x00,                                     // Image size (kept same, optional)
        0x23, 0x2E, 0x00, 0x00,                                     // X pixels per meter
        0x23, 0x2E, 0x00, 0x00,                                     // Y pixels per meter
        0x00, 0x00, 0x00, 0x00,                                     // Colors used
        0x00, 0x00, 0x00, 0x00,                                     // Important colors
        0x00, 0xF8, 0x00, 0x00,                                     // Red mask
        0xE0, 0x07, 0x00, 0x00,                                     // Green mask
        0x1F, 0x00, 0x00, 0x00,                                     // Blue mask
        0x00, 0x00, 0x00, 0x00                                      // Alpha mask (optional, empty)
    };
    (void)bmp800x480;

    const uint8_t bmp1024x600[70] = {
        0x42, 0x4D,             // 'BM'
        0x46, 0xC0, 0x12, 0x00, // File size: 1,228,870
        0x00, 0x00, 0x00, 0x00, // Reserved
        0x46, 0x00, 0x00, 0x00, // Pixel data offset (70)
        0x28, 0x00, 0x00, 0x00, // DIB header size (40)
        0x00, 0x04, 0x00, 0x00, // Width: 1024
        0x58, 0x02, 0x00, 0x00, // Height: 600
        0x01, 0x00,             // Planes
        0x10, 0x00,             // BitCount: 16 (RGB565)
        0x03, 0x00, 0x00, 0x00, // Compression: BI_BITFIELDS
        0x00, 0xC0, 0x12, 0x00, // Image size
        0x23, 0x2E, 0x00, 0x00, // X pixels per meter
        0x23, 0x2E, 0x00, 0x00, // Y pixels per meter
        0x00, 0x00, 0x00, 0x00, // Colors used
        0x00, 0x00, 0x00, 0x00, // Important colors
        0x00, 0xF8, 0x00, 0x00, // Red mask
        0xE0, 0x07, 0x00, 0x00, // Green mask
        0x1F, 0x00, 0x00, 0x00, // Blue mask
        0x00, 0x00, 0x00, 0x00  // Alpha mask
    };
    (void)bmp1024x600;

    File hc = SD_MMC.open("/hardcopy.bmp", "w", true);
#ifdef TFT_LAYOUT_S
    hc.write(bmp320x240, sizeof(bmp320x240));
    uint16_t buff[320];
    for (int i = 240; i > 0; i--) {
        getTFT().readRect(0, i - 1, 320, 1, buff);
        hc.write((uint8_t*)buff, 320 * 2);
    }
    hc.close();
#elifdef TFT_LAYOUT_M
    hc.write(bmp480x320, sizeof(bmp480x320));
    uint16_t buff[480];
    for (int i = 320; i > 0; i--) {
        getTFT().readRect(0, i - 1, 480, 1, buff);
        hc.write((uint8_t*)buff, 480 * 2);
    }
    hc.close();
#elifdef TFT_LAYOUT_L
    #ifdef TFT_ALIGN_LANDSCAPE
    hc.write(bmp800x480, sizeof(bmp800x480));
    uint16_t buff[800];
    for (int i = 480; i > 0; i--) {
        getTFT().readRect(0, i - 1, 800, 1, buff);
        hc.write((uint8_t*)buff, 800 * 2);
    }
    hc.close();
    #elifdef TFT_ALIGN_PORTRAIT
    hc.write(bmp800x480, sizeof(bmp800x480));
    uint16_t row[800];
    for (int bmp_y = 0; bmp_y < 480; bmp_y++) {
        getTFT().readRect(bmp_y, 0, 1, 800, row);
        hc.write((uint8_t*)row, 800 * 2);
    }
    hc.close();
    #endif
#elifdef TFT_LAYOUT_XL
    hc.write(bmp1024x600, sizeof(bmp1024x600));
    uint16_t buff[1024];
    for (int i = 600; i > 0; i--) {
        getTFT().readRect(0, i - 1, 1024, 1, buff);
        hc.write((uint8_t*)buff, 1024 * 2);
    }
    hc.close();
#else

#endif
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void GetRunTimeStats(char* pcWriteBuffer) {
    //     TaskStatus_t* pxTaskStatusArray;
    //     UBaseType_t   uxArraySize;
    //     uint8_t       ulStatsAsPercentage;
    //     uint64_t      ulTotalRunTime;
    //     char          leftSpace[] = "             |";
    //     const size_t  MAX_WRITE_BUFFER_SIZE = 8192; // Assume at least 8KB buffer
    //     size_t        current_len = 0;

    //     // Take a snapshot of the number of tasks in case it changes while this function is executing.
    //     uxArraySize = uxTaskGetNumberOfTasks();

    //     // Allocate a TaskStatus_t structure for each task.  An array could be allocated statically at compile time.
    //     pxTaskStatusArray = (TaskStatus_t*)pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

    //     if (pxTaskStatusArray != NULL) {
    //         // Generate raw status information about each task.
    //         uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, (UBaseType_t)uxArraySize, &ulTotalRunTime);

    //         // For percentage calculations.
    //         ulTotalRunTime /= 100UL;

    //         char* tmpBuff = x_ps_malloc(100);
    //         strlcpy(pcWriteBuffer, leftSpace, MAX_WRITE_BUFFER_SIZE);
    //         current_len = strlen(pcWriteBuffer);
    //         strlcat(pcWriteBuffer, ANSI_ESC_YELLOW " TASKNAME            | RUNTIMECOUNTER | TOTALRUNTIME[%] | CORE | PRIO  |\n", MAX_WRITE_BUFFER_SIZE);
    //         current_len = strlen(pcWriteBuffer);
    //         strlcat(pcWriteBuffer, leftSpace, MAX_WRITE_BUFFER_SIZE);
    //         current_len = strlen(pcWriteBuffer);
    //         strlcat(pcWriteBuffer, "---------------------+----------------+-----------------+------+-------|\n", MAX_WRITE_BUFFER_SIZE);

    //         // Avoid divide by zero errors.
    //         if (ulTotalRunTime > 0) {
    //             // For each populated position in the pxTaskStatusArray array, format the raw data as human readable ASCII data
    //             for (int x = 0; x < uxArraySize; x++) {
    //                 // What percentage of the total run time has the task used? This will always be rounded down to the nearest integer.
    //                 // ulTotalRunTimeDiv100 has already been divided by 100.
    //                 ulStatsAsPercentage = pxTaskStatusArray[x].ulRunTimeCounter / ulTotalRunTime;
    //                 memset(tmpBuff, 0x20, 100);
    //                 memcpy(tmpBuff, pxTaskStatusArray[x].pcTaskName, strlen(pxTaskStatusArray[x].pcTaskName));
    //                 tmpBuff[20] = '|';
    //                 int8_t  core = (pxTaskStatusArray[x].xCoreID);
    //                 uint8_t prio = (pxTaskStatusArray[x].uxBasePriority);
    //                 if (ulStatsAsPercentage) {
    //                     sprintf(tmpBuff + 23, "%12lu  |       %02lu%%       |%4i  |%5d  |", (long unsigned int)pxTaskStatusArray[x].ulRunTimeCounter, (long unsigned int)ulStatsAsPercentage, core,
    //                     prio);
    //                 } else {
    //                     sprintf(tmpBuff + 23, "%12lu  |       <1%%       |%4i  |%5d  |", (long unsigned int)pxTaskStatusArray[x].ulRunTimeCounter, core, prio);
    //                 }
    //                 uint8_t i = 23;
    //                 while (tmpBuff[i] == '0') {
    //                     tmpBuff[i] = ' ';
    //                     i++;
    //                 }
    //                 if (tmpBuff[45] == '0') tmpBuff[45] = ' ';
    //                 current_len = strlen(pcWriteBuffer);
    //                 if (current_len + strlen(leftSpace) + strlen(tmpBuff) + 5 < MAX_WRITE_BUFFER_SIZE) {
    //                     strlcat(pcWriteBuffer, leftSpace, MAX_WRITE_BUFFER_SIZE);
    //                     strlcat(pcWriteBuffer, " ", MAX_WRITE_BUFFER_SIZE);
    //                     strlcat(pcWriteBuffer, tmpBuff, MAX_WRITE_BUFFER_SIZE);
    //                     strlcat(pcWriteBuffer, "\n", MAX_WRITE_BUFFER_SIZE);
    //                 } else {
    //                     log_w("GetRunTimeStats: buffer overflow prevented");
    //                     break; // Stop adding more tasks if buffer is full
    //                 }
    //             }
    //             x_ps_free(&tmpBuff);
    //         }
    //         // The array is no longer needed, free the memory it consumes.
    //         vPortFree(pxTaskStatusArray);

    // #if TFT_CONTROLLER == 7
    //         extern uint64_t s_totalRuntime;
    //         tmpBuff = x_ps_malloc(130);
    //         if (s_totalRuntime > 0) {
    //             sprintf(tmpBuff, "%s" ANSI_ESC_LIGHTGREEN " time since start: %llus, VSYNCS: %llu  ==> fps: %llu", leftSpace, s_totalRuntime, getTFT().getVsyncCounter(),
    //                     getTFT().getVsyncCounter() / s_totalRuntime);
    //         } else {
    //             sprintf(tmpBuff, "%s" ANSI_ESC_LIGHTGREEN " time since start: %llus, VSYNCS: %llu  ==> fps: <1", leftSpace, s_totalRuntime, getTFT().getVsyncCounter());
    //         }
    //         strlcat(tmpBuff, "                                   ", 130);
    //         tmpBuff[90] = '\0';
    //         strlcat(tmpBuff, ANSI_ESC_YELLOW "|\n", 130);
    //         current_len = strlen(pcWriteBuffer);
    //         if (current_len + strlen(tmpBuff) < MAX_WRITE_BUFFER_SIZE) { strlcat(pcWriteBuffer, tmpBuff, MAX_WRITE_BUFFER_SIZE); }
    //         x_ps_free(&tmpBuff);
    // #endif
    //         strlcat(pcWriteBuffer, "             |---------------------+----------------+-----------------+------+-------|\n", MAX_WRITE_BUFFER_SIZE);
    //     }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool get_esp_items(uint8_t* s_resetReason, bool* s_f_FFatFound) {

    ps_ptr<char> chipModel = ESP.getChipModel();
    uint8_t      avMajor = ESP_ARDUINO_VERSION_MAJOR;
    uint8_t      avMinor = ESP_ARDUINO_VERSION_MINOR;
    uint8_t      avPatch = ESP_ARDUINO_VERSION_PATCH;
    printfln(s_tag.sys_info, "ESP32 Chip: {}", chipModel);
    printfln(s_tag.sys_info, "Arduino Version: {}.{}.{}", avMajor, avMinor, avPatch);
    uint8_t idfMajor = ESP_IDF_VERSION_MAJOR;
    uint8_t idfMinor = ESP_IDF_VERSION_MINOR;
    uint8_t idfPatch = ESP_IDF_VERSION_PATCH;
    printfln(s_tag.sys_info, "ESP-IDF Version: {}.{}.{}", idfMajor, idfMinor, idfPatch);
    printfln(s_tag.sys_info, "audioI2S Version: {}", audio.getVersion());
    printfln(s_tag.sys_info, "ARDUINO_LOOP_STACK_SIZE: {} words (32 bit)", CONFIG_ARDUINO_LOOP_STACK_SIZE);
    printfln(s_tag.sys_info, "FLASH size: {} bytes, speed: {} MHz", (long unsigned)ESP.getFlashChipSize(), (long unsigned)ESP.getFlashChipSpeed() / 1000000);
    printfln(s_tag.sys_info, "CPU speed: {} MHz", (long unsigned)ESP.getCpuFreqMHz());
    printfln(s_tag.sys_info, "SDMMC speed: {} MHz", SDMMC_FREQUENCY / 1000000);
#ifdef TFT_MODE_SPI
    printfln(s_tag.sys_info, "TFT speed: {} MHz", TFT_FREQUENCY / 1000000);
#endif

    if (!psramInit()) {
        printfln(s_tag.sys_info, ANSI_ESC_RED "PSRAM not found! MiniWebRadio doesn't work properly without PSRAM!");
    } else {
        printfln(s_tag.sys_info, "PSRAM total size: {} bytes, usable {} bytes", esp_psram_get_size(), ESP.getPsramSize());
    }
    if (ESP.getFlashChipSize() > 80000000) {
        if (!FFat.begin()) {
            if (!FFat.format()) printfln(s_tag.sys_info, "FFat Mount Failed\n");
        } else {
            printfln(s_tag.sys_info, "FFat total space: {} bytes, free space: {} bytes", FFat.totalBytes(), FFat.freeBytes());
            *s_f_FFatFound = true;
        }
    }
    printfln(s_tag.sys_info, "Arduino is pinned to core {}", xPortGetCoreID());
    const char* rr = NULL;
    *s_resetReason = (esp_reset_reason_t)esp_reset_reason();
    switch (*s_resetReason) {
        case ESP_RST_UNKNOWN: rr = "Reset reason can not be determined"; break;
        case ESP_RST_POWERON: rr = "Reset due to power-on event"; break;
        case ESP_RST_EXT: rr = "Reset by external pin (not applicable for ESP32)"; break;
        case ESP_RST_SW: rr = "Software reset via esp_restart"; break;
        case ESP_RST_PANIC: rr = "Software reset due to exception/panic"; break;
        case ESP_RST_INT_WDT: rr = "Reset (software or hardware) due to interrupt watchdog"; break;
        case ESP_RST_TASK_WDT: rr = "Reset due to task watchdog"; break;
        case ESP_RST_WDT:
            rr = "Reset due to other watchdogs";
            *s_resetReason = 1;
            break;
        case ESP_RST_DEEPSLEEP: rr = "Reset after exiting deep sleep mode"; break;
        case ESP_RST_BROWNOUT: rr = "Brownout reset (software or hardware)"; break;
        case ESP_RST_SDIO: rr = "Reset over SDIO"; break;
    }
    printfln(s_tag.sys_info, "RESET_REASON: {}", rr);
    if (chipModel.equals("ESP32-S3")) {
    } // ...  okay
    else if (chipModel.equals("ESP32-P4")) {
    } // ...  okay
    else {
        printfln(s_tag.sys_info, ANSI_ESC_RED "MiniWebRadio does not work with {}", chipModel);
        return false;
    }
    printfln(s_tag.none, "");
    return true;
}

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char ir_buttons_json[] = "[{\"A\":\"0x00\",\"label\":\"IR address\"},"
                               "{\"C\":\"0x4a\",\"label\":\"IR command\"},"
                               "{\"0\":\"0x52\",\"label\":\"ZERO\"},"
                               "{\"10\":\"0x42\",\"label\":\"MUTE\"},"
                               "{\"20\":\"0x40\",\"label\":\"SLEEP\"},"
                               "{\"1\":\"0x16\",\"label\":\"ONE\"},"
                               "{\"11\":\"0x43\",\"label\":\"ARROW RIGHT\"},"
                               "{\"21\":\"0x4a\",\"label\":\"CANCEL\"},"
                               "{\"2\":\"0x19\",\"label\":\"TWO\"},"
                               "{\"12\":\"0x44\",\"label\":\"ARROW LEFT\"},"
                               "{\"22\":\"-1\",\"label\":\"-\"},"
                               "{\"3\":\"0x0d\",\"label\":\"THREE\"},"
                               "{\"13\":\"0x15\",\"label\":\"ARROW DOWN\"},"
                               "{\"4\":\"0x0c\",\"label\":\"FOUR\"},"
                               "{\"14\":\"0x46\",\"label\":\"ARROW UP\"},"
                               "{\"5\":\"0x18\",\"label\":\"FIVE\"},"
                               "{\"15\":\"0x4a\",\"label\":\"MODE\"},"
                               "{\"6\":\"0x5e\",\"label\":\"SIX\"},"
                               "{\"16\":\"0x40\",\"label\":\"OK\"},"
                               "{\"7\":\"0x08\",\"label\":\"SEVEN\"},"
                               "{\"17\":\"0x20\",\"label\":\"-\"},"
                               "{\"8\":\"0x1c\",\"label\":\"EIGHT\"},"
                               "{\"18\":\"-1\",\"label\":\"-\"},"
                               "{\"9\":\"0x5a\",\"label\":\"NINE\"},"
                               "{\"19\":\"-1\",\"label\":\"-\"}]";
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char stations_json[] = "[[\"*\",\"DE\",\"0N 70s\",\"http://0n-70s.radionetz.de:8000/0n-70s.mp3\"],"
                             "[\"*\",\"DE\",\"0N 80s\",\"http://0n-80s.radionetz.de:8000/0n-80s.mp3\"],"
                             "[\"*\",\"DE\",\"0N 90s\",\"http://0n-90s.radionetz.de:8000/0n-90s.mp3\"],"
                             "[\"*\",\"DE\",\"0N Charts\",\"http://0n-charts.radionetz.de:8000/0n-charts.mp3\"],"
                             "[\"*\",\"DE\",\"0N Dance\",\"http://0n-dance.radionetz.de:8000/0n-dance.mp3\"],"
                             "[\"*\",\"DE\",\"0N Disco\",\"http://0n-disco.radionetz.de:8000/0n-disco.mp3\"],"
                             "[\"*\",\"DE\",\"1000 Oldies\",\"http://c3.auracast.net:8010/stream\"],"
                             "[\"*\",\"DE\",\"Eurodance\",\"http://www.laut.fm/eurodance\"],"
                             "[\"\",\"DE\",\"extra-radio 88.0\",\"https://www.extra-radio.de/stream/listen.m3u\"],"
                             "[\"*\",\"DE\",\"Hitradio SKW\",\"http://server4.streamserver24.com:2199/tunein/hitradio.asx\"],"
                             "[\"*\",\"DE\",\"MacSlon's Irish Pub Radio\",\"http://macslons-irish-pub-radio.stream.laut.fm/macslons-irish-pub-radio\"],"
                             "[\"\",\"GR\",\"Άνοιξη 100.7\",\"http://solid1.streamupsolutions.com:55023/stream\"],"
                             "[\"\",\"RU\",\"НАШЕ Радио\",\"http://nashe1.hostingradio.ru/nashe-128.mp3\"],"
                             "[\"\",\"RU\",\"Радио Русские Песни\",\"http://listen.rusongs.ru/ru-mp3-128\"],"
                             "[\"\",\"BG\",\"Свежа България\",\"http://31.13.223.148:8000/fresh.mp3\"],"
                             "[\"\",\"CH\",\"SWISS POP\",\"https://stream.srg-ssr.ch/rsp/aacp_48.asx\"],"
                             "[\"\",\"BG\",\"BGRADIO\",\"http://play.global.audio/bgradio_low.ogg\"],"
                             "[\"\",\"DE\",\"knixx.fm\",\"http://s1.knixx.fm:5347/dein_webradio_vbr.opus\"],"
                             "[\"*\",\"DE\",\"- 0 N - Christmas on Radio\",\"https://0n-christmas.radionetz.de/0n-christmas.aac\"],"
                             "[\"*\",\"GB\",\"BBC 6music\",\"http://as-hls-ww-live.akamaized.net/pool_904/live/ww/bbc_6music/bbc_6music.isml/bbc_6music-audio=96000.norewind.m3u8\"],"
                             "[\"\",\"DE\",\"- 0 N - Movies on Radio\",\"https://0n-movies.radionetz.de/0n-movies.mp3\"],"
                             "[\"*\",\"DE\",\"- 0 N - Top 40 on Radio\",\"https://0n-top40.radionetz.de/0n-top40.mp3\"],"
                             "[\"\",\"DE\",\"ROCKANTENNE Alternative (mp3)\",\"https://stream.rockantenne.de/alternative/stream/mp3\"],"
                             "[\"\",\"PL\",\"Gra Wrocław\",\"http://rmfstream2.interia.pl:8000/radio_gra_wroc\"],"
                             "[\"*\",\"RU\",\"Classic EuroDisco Радио\",\"https://live.radiospinner.com/clsscrdsc-96\"],"
                             "[\"*\",\"DE\",\"Hit Radio FFH - Soundtrack (AAC+)\",\"http://streams.ffh.de/ffhchannels/aac/soundtrack.m3u\"]]";

const char aesKey[] = "mysecretkey12345";
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char voice_time_de[24][50] = {"Beim dritten Ton ist es genau Mitternacht",        "Beim dritten Ton ist es genau ein Uhr",
                                    "Beim dritten Ton ist es genau zwei Uhr",           "Beim dritten Ton ist es genau drei Uhr",
                                    "Beim dritten Ton ist es genau vier Uhr",           "Beim dritten Ton ist es genau fünf Uhr",
                                    "Beim dritten Ton ist es genau sechs Uhr",          "Beim dritten Ton ist es genau sieben Uhr",
                                    "Beim dritten Ton ist es genau acht Uhr",           "Beim dritten Ton ist es genau neun Uhr",
                                    "Beim dritten Ton ist es genau zehn Uhr",           "Beim dritten Ton ist es genau elf Uhr",
                                    "Beim dritten Ton ist es genau zwölf Uhr",          "Beim dritten Ton ist es genau dreizehn Uhr",
                                    "Beim dritten Ton ist es genau vierzehn Uhr",       "Beim dritten Ton ist es genau fünfzehn Uhr",
                                    "Beim dritten Ton ist es genau sechszehn Uhr",      "Beim dritten Ton ist es genau siebzehn Uhr",
                                    "Beim dritten Ton ist es genau achtzehn Uhr",       "Beim dritten Ton ist es genau neunzehn Uhr",
                                    "Beim dritten Ton ist es genau zwanzig Uhr",        "Beim dritten Ton ist es genau einundzwanzig Uhr",
                                    "Beim dritten Ton ist es genau zweiundzwanzig Uhr", "Beim dritten Ton ist es genau dreiundzwanzig Uhr"};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char ir_symbols[34][15] = {"ZERO",        "ONE",        "TWO",        "THREE",     "FOUR",    "FIFE",    "SIX",  "SEVEN",        "EIGHT",    "NINE",    "MUTE",
                                 "ARROW_RIGHT", "ARROW_LEFT", "ARROW_DOWN", "ARROW_UP",  "MODE",    "OKAY",    "N/A",  "PAUSE/RESUME", "STOP",     "ON/OFF",  "RADIO",
                                 "PLAYER",      "DLNA",       "CLOCK",      "OFF_TIMER", "VOLUME+", "VOLUME-", "-30s", "+30s",         "CHANNEL+", "CHANNEL-"};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌  I R _ B U T T O N S  📌📌📌
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class IR_buttons {
  private:
    settings_s* m_settings;
    uint8_t     m_numOfIrButtons = 0;

  public:
    IR_buttons(settings_s* s) {
        m_settings = s;
        m_numOfIrButtons = 0;
    }
    ~IR_buttons() {}

    int16_t hexStringToInt16(const char* str) {
        if (strcmp(str, "-1") == 0) { // Check if it's the special case '-1'
            return -1;                // Special case for unused keys
        }
        // Processing the hexadecimal value
        if (strlen(str) >= 3 && str[0] == '0' && tolower(str[1]) == 'x') {
            return (int16_t)strtol(str, NULL, 16); // Convert Hexadecimal Value
        }
        Serial.println("Invalid format."); // Error output if the format is not correct
        return 0;
    }

    const char* skipWhitespace(const char* str) { // Helper function: Skip spaces
        while (*str && isspace(*str)) { str++; }
        return str;
    }

    const char* validateAndExtractString(const char* ptr, char** dest) { // Error handling: Validation of quoted strings
        if (*ptr == '\"') {
            ptr++; // skip '"'
            const char* start = ptr;
            while (*ptr && *ptr != '\"') { ptr++; }
            if (*ptr == '\"') {
                *dest = strndup(start, ptr - start); // allocate mem for string
                return ptr + 1;                      // skip '"'
            } else {
                Serial.println("Error: Unterminated string.");
                return NULL;
            }
        } else {
            Serial.println("Error: Expected string.");
            return NULL;
        }
    }

    bool parseJSONString(const char* jsonString) { // Function to parse the JSON string
        const char*  ptr = jsonString;
        uint8_t      buttonNr = 0;
        size_t       buttonIndex = 0;
        ps_ptr<char> v;

        // Check if the JSON string starts with '['
        ptr = skipWhitespace(ptr);
        if (*ptr != '[') {
            Serial.println("Error: Expected '[' to start array.");
            return false;
        }
        ptr++; // Überspringe '['

        // Process each JSON object in the array
        char key[16];
        while (*ptr && *ptr != ']' && buttonIndex < 43) {
            ptr = skipWhitespace(ptr);
            if (*ptr == '{') {
                ptr++; // skip '{'
                int16_t val = 0xFF;
                char*   label = NULL;
                bool    validObject = false;

                while (*ptr && *ptr != '}') {
                    ptr = skipWhitespace(ptr);

                    // Schlüssel extrahieren
                    if (*ptr == '\"') {
                        ptr++; // skip '"'
                        const char* keyStart = ptr;
                        while (*ptr && *ptr != '\"') { ptr++; }
                        memset(key, 0, 16);
                        strncpy(key, keyStart, ptr - keyStart);
                        ptr++; // skip '"'
                        ptr = skipWhitespace(ptr);

                        if (*ptr == ':') {
                            ptr++; // skip ':'
                            ptr = skipWhitespace(ptr);
                            // Value based on key
                            if (key[0] == 'A') { // IR Address
                                buttonNr = 42;
                                char* str = NULL;
                                ptr = validateAndExtractString(ptr, &str);
                                if (!ptr) return false;      // error found
                                val = hexStringToInt16(str); // Hex in uint8_t umwandeln
                                x_ps_free(&str);
                                validObject = true;
                            } else if (key[0] == 'C') {
                                ; // IR command unused
                                buttonNr = 43;
                                char* str = NULL;
                                ptr = validateAndExtractString(ptr, &str);
                                if (!ptr) return false;      // error found
                                val = hexStringToInt16(str); // Hex in uint8_t umwandeln
                                x_ps_free(&str);
                                validObject = true;
                            } else if (isdigit(key[0])) { // Nummer, z.B. "0", "10"
                                buttonNr = atoi(key);
                                char* str = NULL;
                                ptr = validateAndExtractString(ptr, &str);
                                if (!ptr) return false;      // error found
                                val = hexStringToInt16(str); // Hex in uint8_t umwandeln
                                x_ps_free(&str);
                                validObject = true;
                            } else if (strcmp(key, "label") == 0) { // Label
                                ptr = validateAndExtractString(ptr, &label);
                                if (!ptr) return false; // error found
                            }
                        }
                    }

                    ptr = skipWhitespace(ptr);
                    if (*ptr == ',') {
                        ptr++; // skip ','
                    }
                }

                // Make sure both values are present
                if (validObject && label != NULL) {
                    m_settings->irbuttons[buttonNr].val = val;
                    m_settings->irbuttons[buttonNr].label = label;
                    // MWR_LOG_WARN("buttonNr {}, val {}, label {}", buttonNr, m_settings->irbuttons[buttonNr].val, m_settings->irbuttons[buttonNr].label);
                    buttonIndex++;
                } else {
                    Serial.println("Error: Invalid object, missing buttonNr or label.");
                    return false;
                }

                ptr = skipWhitespace(ptr);
                if (*ptr == '}') {
                    ptr++; // skip '}'
                }

                ptr = skipWhitespace(ptr);
                if (*ptr == ',') {
                    ptr++; // skip ','
                }
            } else {
                Serial.println("Error: Expected '{' to start an object.");
                return false;
            }
        }

        // Check that the array ends correctly with ']'
        ptr = skipWhitespace(ptr);
        if (*ptr != ']') {
            Serial.println("Error: Expected ']' to close array.");
            return false;
        }
        return true; // JSON parsed successfully
    }

    uint8_t loadButtonsFromJSON(const char* filename) { // Function to load the JSON data
        File file = SD_MMC.open(filename);
        if (!file) {
            Serial.println("Failed to open file");
            return false;
        }
        String jsonString;
        while (file.available()) { jsonString += (char)file.read(); }
        file.close();
        //    MWR_LOG_WARN("{}", jsonString.c_str());
        // JSON parsen
        if (!parseJSONString(jsonString.c_str())) {
            Serial.println("Failed to parse JSON.");
            return false;
        }
        // debug output
        m_numOfIrButtons = 0;
        while (true) {
            if (m_settings->irbuttons[m_numOfIrButtons].label == NULL) break;

            // if(m_settings->irbuttons[m_numOfIrButtons].val == -1) MWR_LOG_WARN("IR_buttonNr {:02}, value -1,   label {}", m_numOfIrButtons, m_settings->irbuttons[m_numOfIrButtons].label);
            //  else MWR_LOG_WARN("IR_buttonNr {:02}, value 0x{:02X}, label {}", m_numOfIrButtons, m_settings->irbuttons[m_numOfIrButtons].val, m_settings->irbuttons[m_numOfIrButtons].label);
            m_numOfIrButtons++;
        }
        m_settings->numOfIrButtons = m_numOfIrButtons;
        return m_numOfIrButtons;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌  S D _ C O N T E N T   📌📌📌
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class SD_content {
  private:
    struct FileInfo {
        int32_t      fileSize;
        ps_ptr<char> fileName;
        ps_ptr<char> filePath;

        FileInfo(int32_t fs, ps_ptr<char> fn, ps_ptr<char> fp) : fileSize(fs), fileName(fn), filePath(fp) {}

        ~FileInfo() = default;
        FileInfo(const FileInfo&) = default;
        FileInfo& operator=(const FileInfo&) = default;
    };
    std::vector<FileInfo> m_files;
    ps_ptr<char>          m_buff;
    ps_ptr<char>          m_lastConnectedFile = "";
    ps_ptr<char>          m_lastConnectedFolder = "";
    ps_ptr<char>          m_lastConnectedFileName = "";
    ps_ptr<char>          m_JSONstr;

  public:
    SD_content() { m_files.clear(); }
    ~SD_content() {
        m_files.clear();
        m_JSONstr.reset();
        m_lastConnectedFile.reset();
        m_lastConnectedFolder.reset();
        m_lastConnectedFileName.reset();
    }

    bool listFilesInDir(ps_ptr<char> path, boolean audioFilesOnly, boolean withoutDirs) {
        m_files.clear();

        if (!SD_MMC.exists(path.c_get())) {
            printfln(s_tag.sd_card, ANSI_ESC_RED "SD_MMC/{} not exist", path);
            return false;
        }

        File masterFile = SD_MMC.open(path.c_get());

        if (!masterFile || !masterFile.isDirectory()) {
            printfln(s_tag.sd_card, ANSI_ESC_RED "SD_MMC/{} is not a directory", path);
            masterFile.close();
            return false;
        }

        while (true) {
            File slaveFile = masterFile.openNextFile();
            if (!slaveFile) break;

            if (slaveFile.isDirectory()) {
                if (!withoutDirs) {
                    ps_ptr<char> filePath = slaveFile.path();
                    filePath.append("/");

                    m_files.emplace_back(-1, slaveFile.name(), filePath);
                }
            } else {
                if (!audioFilesOnly ||                     //
                    endsWith(slaveFile.name(), ".mp3") ||  //
                    endsWith(slaveFile.name(), ".aac") ||  //
                    endsWith(slaveFile.name(), ".m4a") ||  //
                    endsWith(slaveFile.name(), ".wav") ||  //
                    endsWith(slaveFile.name(), ".m3u") ||  //
                    endsWith(slaveFile.name(), ".flac") || //
                    endsWith(slaveFile.name(), ".opus") || //
                    endsWith(slaveFile.name(), ".ogg")) {  //

                    m_files.emplace_back( //
                        slaveFile.size(), //
                        slaveFile.name(), //
                        slaveFile.path()  //
                    );
                }
            }
            slaveFile.close();
        }
        sort();
        masterFile.close();
        return true;
    }

    bool isDir(uint16_t idx) {
        if (idx >= m_files.size()) {
            MWR_LOG_ERROR("idx {} is oor, max = {}", idx, m_files.size() - 1);
            return false;
        }

        return m_files[idx].fileSize == -1; // -1 means dir
    }

    size_t getSize() { return m_files.size(); }

    ps_ptr<char> getColouredSStringByIndex(uint16_t idx) {
        if (m_files.size() == 0) {
            MWR_LOG_WARN("m_files.size() is 0");
            return "";
        }
        if (m_files.size() < idx + 1) {
            MWR_LOG_WARN("idx {} is oor, max = {}", idx, m_files.size());
            return "";
        }
        if (m_files[idx].fileSize == -1) return m_files[idx].fileName;                            // directory
        m_buff.assignf("{}" ANSI_ESC_YELLOW " {}", m_files[idx].fileName, m_files[idx].fileSize); // file
        return m_buff;
    }

    ps_ptr<char> getFileNameByIndex(uint16_t idx) {
        if (idx >= m_files.size()) {
            MWR_LOG_WARN("idx {} is oor, size = {}", idx, m_files.size());
            return "";
        }
        return m_files[idx].fileName;
    }

    int32_t getFileSizeByIndex(uint16_t idx) {
        if (idx >= m_files.size()) {
            MWR_LOG_WARN("idx {} is oor, size = {}", idx, m_files.size());
            return 0;
        }
        return m_files[idx].fileSize; // returns -1 for dirs
    }

    ps_ptr<char> getFilePathByIndex(uint16_t idx) {
        if (idx >= m_files.size()) {
            MWR_LOG_WARN("idx {} is oor, size = {}", idx, m_files.size());
            return "";
        }
        /*
            dir_a
                dir_b
                    file_a
                    file_b
                file_c
                file_d

            getFilePathByIndex(0) returns "/dir_a/"
            getFilePathByIndex(3) returns "/dir_a/dir_b/file_b"
            getFilePathByIndex(5) returns "/dir_a/file_d"
        */
        return m_files[idx].filePath;
    }

    ps_ptr<char> getFileFolderByIndex(uint16_t idx) {
        if (idx >= m_files.size()) {
            MWR_LOG_WARN("idx {} is oor, size = {}", idx, m_files.size());
            return "";
        }
        /*
            dir_a
                dir_b
                    file_a
                    file_b
                file_c
                file_d

            getFileFolderByIndex(0) returns "/dir_a/"
            getFileFolderByIndex(1) returns "/dir_a/dir_b/"
            getFileFolderByIndex(5) returns "/dir_a/"
        */
        if (isDir(idx)) return m_files[idx].filePath;
        int lastSlashIndex = m_files[idx].filePath.last_index_of('/');
        m_buff = m_files[idx].filePath;
        m_buff[lastSlashIndex + 1] = '\0';
        return m_buff;
    }

    int16_t getIndexByName(ps_ptr<char> path) {
        /*
            dir_a
                dir_b
                    file_a
                    file_b
                file_c
                file_d

            getIndexByName("/dir_a") returns 0
            getIndexByName("/dir_a/dir_b/file_b") returns 3
            getIndexByName("/dir_a/dir_b/file_y") returns -1
        */
        if (!path.valid()) return -1;
        for (int i = 0; i < m_files.size(); i++) {
            if (m_files[i].filePath.equals(path)) { return i; }
        }
        return -1;
    }

    uint16_t getNextAudioFile(uint16_t currIdx) { // assume listFilesInDir with "audioFilesOnly"
        if (m_files.empty()) return 0;
        if (currIdx >= m_files.size()) currIdx = m_files.size() - 1;
        uint16_t newIdx = currIdx;
        while (true) {
            newIdx++;
            if (newIdx >= m_files.size()) newIdx = 0;
            if (newIdx == currIdx) break;                           // avoid an infinite loop
            if (!m_files[newIdx].fileName.ends_with(".m3u")) break; // skip m3u files
        }
        return newIdx;
    }

    uint16_t getPrevAudioFile(uint16_t currIdx) { // assume listFilesInDir with "audioFilesOnly"
        if (m_files.empty()) return 0;
        if (currIdx >= m_files.size()) currIdx = m_files.size() - 1;
        uint16_t newIdx = currIdx;
        while (true) {
            if (newIdx == 0) {
                newIdx = m_files.size() - 1;
            } else {
                --newIdx;
            }
            if (newIdx == currIdx) break;                           // avoid an infinite loop
            if (!m_files[newIdx].fileName.ends_with(".m3u")) break; // skip m3u files
        }
        return newIdx;
    }

    void setLastConnectedFile(ps_ptr<char> lastconnectedItem) {
        /*  lastconnectedItem                       m_lastConnectedFolder       m_lastConnectedFileName     m_lastConnectedFile
            "xyz/chicken.wav"                       "/audiofiles/"              {}                          "/audiofiles/"                      // does not start with "/"
            "/audiofiles/wavfiles/chickenwav"       "/audiofiles/wavfiles/"     {}                          "/audiofiles/wavfiles/"             // file has no extension
            "/audiofiles/wavfiles/.wav"             "/audiofiles/wavfiles/"     {}                          "/audiofiles/wavfiles/"             // file has no name
            "/chicken.wav"                          "/"                         "chicken.wav"               "/chicken.wav"                      // we have no folder
            "/audiofiles/wavfiles/"                 "/audiofiles/wavfiles/"     {}                          "/audiofiles/wavfiles/"             // we have no file
            "/audiofiles/wavfiles/chicken.wav"      "/audiofiles/wavfiles/"     "chicken.wav"               "/audiofiles/wavfiles/chicken.wav"
        */
        int posFirst = 0, posLast = 0, posDot = 0;

        if (!lastconnectedItem.valid()) { goto exit; } // guard, lastconnectedItem == NULL

        posFirst = lastconnectedItem.index_of("/", 0);
        if (posFirst != 0) { // guard, does not start with /
            m_lastConnectedFolder.assign("/audiofiles/");
            m_lastConnectedFileName.reset();
            goto exit;
        }

        posLast = lastconnectedItem.last_index_of('/');
        if (posFirst == posLast) { // we have no folder
            m_lastConnectedFolder = "/";
        } else {
            m_lastConnectedFolder = lastconnectedItem.substr(0, posLast + 1);
        }

        posDot = lastconnectedItem.index_of('.', posLast);
        if (posDot == -1) { // file has no extension
            m_lastConnectedFileName.reset();
        } else {
            if (posDot == posLast + 1) {
                m_lastConnectedFileName.reset(); // extension without name
            }
            m_lastConnectedFileName = lastconnectedItem.substr(posLast + 1); // fileNane exists
        }

    exit:
        m_lastConnectedFile.clone_from(m_lastConnectedFolder);
        m_lastConnectedFile.append(m_lastConnectedFileName);
        MWR_LOG_DEBUG("lastconnectedItem {}", lastconnectedItem);
        MWR_LOG_DEBUG("m_lastConnectedFolder {}", m_lastConnectedFolder);
        MWR_LOG_DEBUG("m_lastConnectedFileName {}", m_lastConnectedFileName);
        MWR_LOG_DEBUG("lastConnectedFile {}", m_lastConnectedFile);

        listFilesInDir(m_lastConnectedFolder, true, false);
    }

    ps_ptr<char> getLastConnectedFolder() { return m_lastConnectedFolder; }
    ps_ptr<char> getLastConnectedFileName() { return m_lastConnectedFileName; }

    int16_t getPosByFileName(ps_ptr<char> fileName) {
        for (size_t i = 0; i < m_files.size(); i++) {
            if (m_files[i].fileName == fileName) return i; // fileName e.g. "file.mp3"
        }
        return -1;
    }

    ps_ptr<char> stringifyDirContent(ps_ptr<char> path) {
        /*
                Music/
                Pictures/
                song.mp3
                test.flac

                [
                  {"name":"Music","dir":true,"size":0},
                  {"name":"Pictures","dir":true,"size":0},
                  {"name":"song.mp3","dir":false,"size":123456},
                  {"name":"test.flac","dir":false,"size":654321}
                ]
        */

        if (!listFilesInDir(path, false, false)) return "[]";
        m_JSONstr.assign("[");
        bool first = true;
        for (size_t i = 0; i < m_files.size(); ++i) {
            if (m_files[i].fileName.starts_with(".")) continue;
            if (!first) m_JSONstr.append(",");
            first = false;
            m_JSONstr.append("{\"name\":\"");
            m_JSONstr.append(m_files[i].fileName);
            m_JSONstr.append("\",\"dir\":");
            if (isDir(i)) {
                m_JSONstr.append("true,\"size\":0");
            } else {
                m_JSONstr.appendf("false,\"size\":{}", m_files[i].fileSize);
            }
            m_JSONstr.append("}");
        }
        m_JSONstr.append("]");
        return m_JSONstr;
    }

  private:
    void sort() {
        std::sort(m_files.begin(), m_files.end(), [](const FileInfo& a, const FileInfo& b) {
            // Zuerst nach Ordner vs. Datei sortieren
            if (a.fileSize == -1 && b.fileSize != -1) {
                return true; // a ist Ordner, b ist Datei
            }
            if (a.fileSize != -1 && b.fileSize == -1) {
                return false; // a ist Datei, b ist Ordner
            }
            // Wenn beide entweder Ordner oder beide Dateien sind, alphabetisch sortieren
            return strcmp(a.fileName.get(), b.fileName.get()) < 0;
        });
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌  S T A T I O N S M A N A G E M E N T    📌📌📌
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class stationManagement {
  private:
    struct sta {
        std::vector<uint8_t>  fav;
        std::vector<uint16_t> favStaNr;
        std::vector<char*>    country;
        std::vector<char*>    name;
        std::vector<char*>    url;
    } m_stations;

    uint16_t  m_staCnt = 0;
    uint16_t  m_staFavCnt = 0;
    uint16_t* m_curStation = 0;

  public:
    stationManagement(uint16_t* curStation) {
        clearStations();
        m_curStation = curStation;
    }
    ~stationManagement() { clearStations(); }

  private:
    void clearStations() {
        vector_clear_and_shrink(m_stations.country);
        vector_clear_and_shrink(m_stations.name);
        vector_clear_and_shrink(m_stations.url);
        m_stations.fav.clear();
        m_stations.fav.shrink_to_fit();
        m_stations.favStaNr.clear();
        m_stations.favStaNr.shrink_to_fit();

        m_stations.country.push_back(x_ps_strdup("unknown"));
        m_stations.name.push_back(x_ps_strdup("unknown"));
        m_stations.url.push_back(x_ps_strdup("unknown"));
        m_stations.fav.push_back('0');
        m_stations.favStaNr.push_back(0);
    }

  public:
    void updateStationsList() {
        clearStations();
        uint8_t item = 0;
        m_staCnt = 0;
        m_staFavCnt = 0;
        if (!SD_MMC.exists("/stations.json")) { return; }
        char buff[1024];
        File file = SD_MMC.open("/stations.json");
        while (file.available()) {
            char c = file.read();
            if (c == '[' || c == ']' || c == ',' || c == '\n' || c == '\r') { continue; } // skip
            if (c == '\"') {                                                              // start of string
                int pos = file.readBytesUntil('\"', buff, 1024);
                buff[pos] = 0;

                if (item == 0) {
                    m_stations.fav.push_back(buff[0]);
                    m_staCnt++;
                    if (buff[0] == '*') {
                        m_staFavCnt++;
                        m_stations.favStaNr.push_back(m_staCnt);
                    }
                }
                if (item == 1) { m_stations.country.push_back(x_ps_strdup(buff)); }
                if (item == 2) { m_stations.name.push_back(x_ps_strdup(buff)); }
                if (item == 3) { m_stations.url.push_back(x_ps_strdup(buff)); }
                item++;
                if (item > 3) item = 0;
                if (m_staCnt > 999) break;
            }
        }
        file.close();
    }
    //----------------------------------------------------------------------------------------------------------
    uint16_t getCurrentStationNumber() { return *m_curStation; }
    //----------------------------------------------------------------------------------------------------------
    uint16_t getSumStations() { return m_staCnt; }
    //----------------------------------------------------------------------------------------------------------
    uint16_t getSumFavStations() { return m_staFavCnt; }
    //----------------------------------------------------------------------------------------------------------
    uint16_t nextStation() {
        if (!m_staCnt) return 1;
        (*m_curStation)++;
        if (*m_curStation > m_staCnt) *m_curStation = 1;
        return *m_curStation;
    }
    //----------------------------------------------------------------------------------------------------------
    uint16_t nextFavStation() {
        if (!m_staCnt) return 1;
        uint16_t cnt = 0;
        int16_t  tmp = (*m_curStation);
        while (true) {
            tmp++;
            cnt++;
            if (cnt > m_staCnt) break;
            if (tmp > m_staCnt) tmp = 1;
            if (m_stations.fav[tmp] == '*') {
                *m_curStation = tmp;
                break;
            }
        }
        return *m_curStation;
    }
    //----------------------------------------------------------------------------------------------------------
    uint16_t prevStation() {
        if (!m_staCnt) return 1;
        (*m_curStation)--;
        if (*m_curStation < 1) *m_curStation = m_staCnt;
        return *m_curStation;
    }
    //----------------------------------------------------------------------------------------------------------
    uint16_t prevFavStation() {
        if (!m_staCnt) return 1;
        uint16_t cnt = 0;
        int16_t  tmp = (*m_curStation);
        while (true) {
            tmp--;
            cnt++;
            if (cnt > m_staCnt) break;
            if (tmp < 1) tmp = m_staCnt;
            if (m_stations.fav[tmp] == '*') {
                *m_curStation = tmp;
                break;
            }
        }
        return *m_curStation;
    }
    //----------------------------------------------------------------------------------------------------------
    uint16_t setStationByNumber(uint16_t staNr) {
        if (!m_staCnt) return 1;
        if (staNr > m_staCnt)
            *m_curStation = m_staCnt;
        else if (staNr == 0) {
            *m_curStation = 1;
        } else {
            *m_curStation = staNr;
        }
        return *m_curStation;
    }
    //----------------------------------------------------------------------------------------------------------
    ps_ptr<char> getStationName(uint16_t staNr) {
        if (staNr > m_staCnt) return {};
        if (!m_stations.name[staNr]) return {};
        return m_stations.name[staNr];
    }
    char getStationFav(uint16_t staNr) { // 0 = not fav, * = fav, 1..3 = fav number (notused)
        if (staNr > m_staCnt) return '0';
        if (!m_stations.fav[staNr]) return '0';
        return m_stations.fav[staNr];
    }
    const char* getStationUrl(uint16_t staNr) {
        if (staNr > m_staCnt) return strdup("unknown");
        if (!m_stations.url[staNr]) return strdup("unknown");
        return m_stations.url[staNr];
    }
    const char* getStationCountry(uint16_t staNr) {
        if (staNr > m_staCnt) return strdup("unknown");
        if (!m_stations.country[staNr]) return strdup("unknown");
        return m_stations.country[staNr];
    }
};

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌   P L A Y L I S T     📌📌📌
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Playlist {

  public:
    Playlist() {}
    ~Playlist() {}

  private:
    ps_ptr<char>        m_playlist_path = {};
    deque<ps_ptr<char>> m_content_file = {};
    deque<ps_ptr<char>> m_content_items = {};
    File                m_playlist_file;
    int16_t             m_index = -1;

    void reset() {
        m_playlist_path.clear();
        m_content_file.clear();
        m_content_items.clear();
        m_playlist_file.close();
        m_index = -1;
    }

    boolean isAudio(File file) {
        if (endsWith(file.name(), ".mp3") || endsWith(file.name(), ".aac") || endsWith(file.name(), ".m4a") || endsWith(file.name(), ".wav") || endsWith(file.name(), ".flac") ||
            endsWith(file.name(), ".opus") || endsWith(file.name(), ".ogg")) {
            return true;
        }
        return false;
    }

    boolean isAudio(ps_ptr<char> file) {
        if (file.ends_with(".mp3") || file.ends_with(".aac") || file.ends_with(".m4a") || file.ends_with(".wav") || file.ends_with(".flac") || file.ends_with(".opus") || file.ends_with(".ogg")) {
            return true;
        }
        return false;
    }

  public:
    bool create_playlist_from_file(ps_ptr<char> path) {
        reset();

        if (!path.valid()) return false;

        if (!path.ends_with(".m3u")) {
            MWR_LOG_ERROR("wrong playlist path {}", path);
            return false;
        }

        if (!SD_MMC.exists(path.get())) {
            MWR_LOG_ERROR("Playlistfile {} not found", path);
            return false;
        }

        m_playlist_file = SD_MMC.open(path.get());

        if (m_playlist_file.size() > 1048576) {
            MWR_LOG_ERROR("Playlist too big, size is {}", m_playlist_file.size());
            reset();
            return false;
        }

        int idx = path.last_index_of('/');
        if (idx != 0) {
            m_playlist_path = path.substr(0, idx + 1);
        } else {
            m_playlist_path = path;
        }
        ps_ptr<char> readBuff;
        ps_ptr<char> itemName;
        ps_ptr<char> itemPath;

        readBuff.alloc(2024);
        itemName.alloc(2024);
        itemPath.alloc(2024);

        bool hasExtInfo = false;

        while (m_playlist_file.available()) {

            size_t bytesRead = m_playlist_file.readBytesUntil('\n', readBuff.get(), readBuff.size());

            if (bytesRead < 1) continue;

            readBuff[bytesRead] = '\0';
            trim(readBuff.get());

            if (readBuff.empty()) continue;                // blank line
            if (readBuff.starts_with("#EXTM3U")) continue; // #EXTM3U

            // #EXTINF
            if (readBuff.starts_with("#EXTINF:")) {
                itemName = readBuff.substr(8);
                int comma = itemName.index_of(',');
                if (comma >= 0) {
                    auto duration = itemName.substr(0, comma);
                    int  d = duration.to_int32();
                    itemName = itemName.substr(comma + 1);                                               // title after the comma
                    if (d > 0) { itemName.appendf(" " ANSI_ESC_YELLOW "{}s" ANSI_ESC_RESET, duration); } // Optionally append duration
                }
                hasExtInfo = true;
                continue;
            }

            // Ignore other comments
            if (readBuff.starts_with("#")) continue;

            // -----------------------------------------
            // Here’s a proper playlist entry
            // -----------------------------------------

            if (readBuff.starts_with("file://")) {
                itemPath = readBuff.substr(7);
                itemPath.urldecode();
            } else if (readBuff.starts_with_icase("http://") || readBuff.starts_with_icase("https://")) {
                itemPath = readBuff;
            } else {
                if (!readBuff.starts_with("/")) {
                    itemPath = m_playlist_path + readBuff;
                } else {
                    itemPath = readBuff;
                }
                itemPath.urldecode();
            }

            // Import audio files only
            if (!isAudio(itemPath)) continue;

            // No EXTINF available:
            // Use the file name as the title
            if (!hasExtInfo) {
                int slash = itemPath.last_index_of('/');
                if (slash >= 0) {
                    itemName = itemPath.substr(slash + 1);
                } else {
                    itemName = itemPath;
                }
            }
            m_content_items.push_back(itemName);
            m_content_file.push_back(itemPath);

            // EXTINF applies only to the next entry exactly
            hasExtInfo = false;
            itemName.clear();
            itemPath.clear();
        }
        m_playlist_file.close();
        return true;
    }

    bool create_playlist_from_SD_folder(ps_ptr<char> path) { // all files within a SD folder
        reset();
        if (!SD_MMC.exists(path.get())) {
            MWR_LOG_ERROR("SD_MMC/{} not exist", path);
            return false;
        }
        File folder = SD_MMC.open(path.get());
        if (!folder.isDirectory()) {
            MWR_LOG_ERROR("SD_MMC{} is not a directory", path);
            folder.close();
            return false;
        }
        m_content_file.clear();  // clear path first
        m_content_items.clear(); // clear name first

        while (true) { // get content
            File file = folder.openNextFile();
            if (!file) break;
            if (file.isDirectory()) continue;
            if (isAudio(file)) {
                m_content_file.push_back(file.path());
                ps_ptr<char> name;
                name = file.name();
                name.appendf("" ANSI_ESC_YELLOW " {}" ANSI_ESC_RESET "", file.size());
                m_content_items.push_back(name);
            }
            file.close();
        }
        folder.close();

        // for (int i = 0; i < m_content_file.size(); i++) {
        //     MWR_LOG_WARN("{}, {}", i, m_content_file[i]);  // path
        //     MWR_LOG_INFO("{}, {}", i, m_content_items[i]); // name
        // }
        return true;
    }

    bool create_playlist_from_DLNA_folder(const std::deque<DLNA_Client::srvItem>* foldercontent) {
        reset();
        for (int i = 0; i < foldercontent->size(); i++) {
            MWR_LOG_DEBUG("{} : ({}) {} {} {}", i, foldercontent->at(i).isAudio, foldercontent->at(i).itemURL, foldercontent->at(i).title, foldercontent->at(i).duration);
            if (!foldercontent->at(i).isAudio) continue;
            uint16_t     itemURL_len = foldercontent->at(i).itemURL.strlen();
            uint16_t     title_len = foldercontent->at(i).title.strlen();
            uint16_t     duration_len = foldercontent->at(i).duration.strlen();
            uint16_t     len = itemURL_len + title_len + duration_len + 3;
            ps_ptr<char> itstr(len);
            itstr = foldercontent->at(i).itemURL;
            itstr += "\n";
            itstr += foldercontent->at(i).duration;
            itstr += ",";
            itstr += foldercontent->at(i).title;
            MWR_LOG_DEBUG("pushing to playlist : {}", itstr);
            m_content_file.push_back(itstr);
        }
        if (!m_content_file.size()) return false;
        MWR_LOG_INFO("pls length {}", m_content_file.size());
        return true;
    }

    void sort_alphabetical() {
        for (size_t i = 0; i < m_content_file.size(); i++) {
            for (size_t j = 1; j < m_content_file.size() - i; j++) {
                if (m_content_file[j - 1] > m_content_file[j]) {
                    swap(m_content_file[j - 1], m_content_file[j]);
                    swap(m_content_items[j - 1], m_content_items[j]);
                }
            }
        }
    }

    void sort_random() {
        if (m_content_file.size() < 2) return;

        for (size_t i = 0; i < m_content_file.size(); i++) {
            size_t randIndex = random(0, m_content_file.size());

            m_content_file[i].swap(m_content_file[randIndex]);
            m_content_items[i].swap(m_content_items[randIndex]);
        }
    }

    int16_t next_index() {
        if ((m_index + 1) == m_content_file.size()) return -1;
        m_index++;
        return m_index;
    }

    int16_t previous_index() {
        if (m_index == -1) return -1;
        m_index--;
        return m_index;
    }

    ps_ptr<char> get_file_by_index(uint16_t idx) {
        ps_ptr<char> s = "";
        if (idx < m_content_file.size()) s = m_content_file[idx];
        return s;
    }

    ps_ptr<char> get_items_by_index(uint16_t idx) {
        ps_ptr<char> s = "";
        if (idx < m_content_items.size()) s = m_content_items[idx];
        return s;
    }

    ps_ptr<char> get_file() {
        ps_ptr<char> s = "";
        if (m_index == -1) { return s; }
        if (m_index >= m_content_file.size()) { return s; }
        s = m_content_file[m_index];
        return s;
    }

    ps_ptr<char> get_items() {
        ps_ptr<char> s = "";
        if (m_index == -1) { return s; }
        if (m_index >= m_content_items.size()) { return s; }
        s = m_content_items[m_index];
        return s;
    }

    ps_ptr<char> get_coloured_file() {
        ps_ptr<char> s = "";
        if (m_index != -1) s.assignf(ANSI_ESC_CYAN "{}" ANSI_ESC_RESET, m_content_file[m_index]);
        s.println();
        return s;
    }

    ps_ptr<char> get_coloured_index() {
        ps_ptr<char> s = "";
        if (m_index != -1) s.assignf(ANSI_ESC_ORANGE "{:03}/{:03}" ANSI_ESC_RESET, m_index + 1, m_content_file.size());
        return s;
    }

    uint16_t get_size() { return m_content_file.size(); }
};

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌   R E C O R D E R     📌📌📌
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

struct WAVHeader {
    char     riff[4] = {'R', 'I', 'F', 'F'};
    uint32_t size;
    char     wave[4] = {'W', 'A', 'V', 'E'};
    char     fmt[4] = {'f', 'm', 't', ' '};
    uint32_t fmtSize = 16;
    uint16_t format = 1;
    uint16_t channels = 2;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bits;
    char     data[4] = {'d', 'a', 't', 'a'};
    uint32_t dataSize;
};

constexpr size_t REC_BUFFER_SIZE = 512 * 1024; // 512KB für 2-3 Sekunden Puffer
constexpr size_t WRITE_CHUNK_SIZE = 1024;      // not too big!
constexpr size_t SD_FLUSH_INTERVAL = 65536;    // Alle 64KB flush
ps_ptr<uint8_t>  rec_buffer;
ps_ptr<uint8_t>  writeBuffer;

class AudioRecorder {
  public:
    std::atomic<size_t> writePos{0};
    std::atomic<size_t> readPos{0};
    uint32_t            totalBytes = 0;
    uint16_t            sampleRate = 44100;
    uint32_t            overflowCount = 0;

    volatile bool startRequested = false;
    volatile bool stopRequested = false;
    volatile bool running = false;

    bool push16(const int32_t* data, size_t frames) {
        // frames = Stereo-Frames
        size_t bytes16 = frames * 2 * sizeof(int16_t);

        size_t currentWrite = writePos.load(std::memory_order_relaxed);
        size_t currentRead = readPos.load(std::memory_order_acquire);

        size_t free = (currentRead + REC_BUFFER_SIZE - currentWrite - 1) % REC_BUFFER_SIZE;

        if (bytes16 > free) {
            overflowCount++;
            return false;
        }

        for (size_t i = 0; i < frames * 2; i++) {
            // 32 → 16 Bit (High word)
            int32_t v = data[i] >> 16;

            // Optional Clipping (sicher)
            if (v > 32767) v = 32767;
            if (v < -32768) v = -32768;

            int16_t s = (int16_t)v;

            // Write byte by byte (LE)
            rec_buffer[currentWrite] = s & 0xFF;
            currentWrite = (currentWrite + 1) % REC_BUFFER_SIZE;
            rec_buffer[currentWrite] = (s >> 8) & 0xFF;
            currentWrite = (currentWrite + 1) % REC_BUFFER_SIZE;
        }

        writePos.store(currentWrite, std::memory_order_release);
        return true;
    }

    // Copies data to dest, returns bytes actually read
    size_t pop(uint8_t* dest, size_t maxLen) {
        size_t currentRead = readPos.load(std::memory_order_relaxed);
        size_t currentWrite = writePos.load(std::memory_order_acquire);

        if (currentRead == currentWrite) return 0;

        size_t avail = (currentWrite > currentRead) ? (currentWrite - currentRead) : (REC_BUFFER_SIZE - currentRead);

        size_t toRead = std::min(avail, maxLen);

        // Wrap-around handling
        size_t firstChunk = std::min(toRead, REC_BUFFER_SIZE - currentRead);
        memcpy(dest, &rec_buffer[currentRead], firstChunk);
        if (toRead > firstChunk) { memcpy(dest + firstChunk, &rec_buffer[0], toRead - firstChunk); }

        readPos.store((currentRead + toRead) % REC_BUFFER_SIZE, std::memory_order_release);
        return toRead;
    }

    // For external access to buffers (e.g. for pop with pointer math, but not recommended)
    size_t available() {
        size_t w = writePos.load(std::memory_order_acquire);
        size_t r = readPos.load(std::memory_order_acquire);
        return (w >= r) ? (w - r) : (REC_BUFFER_SIZE - r + w);
    }
};

AudioRecorder recorder;

void wavWriterTask(void*) {
    File      file;
    WAVHeader hdr;
    bool      fileOpen = false;

    size_t   writeBufferFill = 0;
    uint32_t bytesSinceFlush = 0;

    while (true) {
        // --- START REQUEST ---
        if (recorder.startRequested && !fileOpen) {
            recorder.startRequested = false;

            // Datei mit Zeitstempel erstellen
            char filename[64];
            snprintf(filename, sizeof(filename), "/audiofiles/recording.wav");

            file = SD_MMC.open(filename, FILE_WRITE);
            if (!file) {
                MWR_LOG_ERROR("Failed to open file! \"/audiofiles/recording.wav\"");
                continue;
            }

            // prepeare header
            hdr.sampleRate = recorder.sampleRate;
            hdr.byteRate = recorder.sampleRate * 2 * 2; // Stereo, 16-bit
            hdr.blockAlign = 2 * 2;                     // 8 bytes per frame
            hdr.bits = 16;
            hdr.dataSize = 0;
            hdr.size = 36; // 44 - 8 (RIFF header)

            file.write((uint8_t*)&hdr, sizeof(hdr));
            recorder.totalBytes = 0;
            writeBufferFill = 0;
            bytesSinceFlush = 0;
            fileOpen = true;
            recorder.running = true;
            rec_buffer.clear();
            writeBuffer.clear();
            printfln(s_tag.recorder, ANSI_ESC_YELLOW "Recording started: " ANSI_ESC_YELLOW "{}", filename);
        }

        // --- WRITE DATA ---
        if (fileOpen) {
            // fill local buffer
            while (writeBufferFill < WRITE_CHUNK_SIZE) {
                size_t spaceInLocalBuffer = WRITE_CHUNK_SIZE - writeBufferFill;
                size_t bytesRead = recorder.pop(writeBuffer + writeBufferFill, spaceInLocalBuffer);

                if (bytesRead == 0) break; // ringbuffer is empty

                writeBufferFill += bytesRead;
            }

            // write full block to SD
            if (writeBufferFill >= WRITE_CHUNK_SIZE) {
                size_t written = file.write(writeBuffer.get(), WRITE_CHUNK_SIZE);
                if (written != WRITE_CHUNK_SIZE) {
                    MWR_LOG_ERROR("SD write error!");
                    // Optional: Fehlerbehandlung, Buffer zurückhalten?
                }

                recorder.totalBytes += written;
                bytesSinceFlush += written;
                writeBufferFill = 0; // buffer is empty (or move remaining data)

                // Periodischer Flush für Datenintegrität
                if (bytesSinceFlush >= SD_FLUSH_INTERVAL) {
                    file.flush();
                    bytesSinceFlush = 0;
                }
            }
        }

        // --- STOP REQUEST ---
        if (recorder.stopRequested && fileOpen) {
            recorder.stopRequested = false;

            // Write remaining data to local buffer
            if (writeBufferFill > 0) {
                file.write(writeBuffer.get(), writeBufferFill);
                recorder.totalBytes += writeBufferFill;
            }

            // Update header
            hdr.dataSize = recorder.totalBytes;
            hdr.size = recorder.totalBytes + 36;

            file.seek(0);
            file.write((uint8_t*)&hdr, sizeof(hdr));
            file.flush();
            file.close();

            fileOpen = false;
            writeBufferFill = 0;
            recorder.running = false;
            printfln(s_tag.recorder, "Recording stopped. Total bytes: " ANSI_ESC_CYAN "{}" ANSI_ESC_RESET ", Overflows: " ANSI_ESC_CYAN "{}", recorder.totalBytes, recorder.overflowCount);
        }

        // Small delay to feed watchdog and release CPU
        // But not too long, so that the ring buffer does not overflow!
        vTaskDelay(pdMS_TO_TICKS(1)); // 1ms = ~176 Bytes bei 44.1kHz Stereo 32-bit
    }
}

void audio_process_raw_samples(int32_t* outBuff, int16_t validSamples) {
    if (recorder.running == true) { recorder.push16(outBuff, validSamples); }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char timezones_json[] = "[[\"Africa/Accra\",\"GMT0\"],"
                              "[\"Africa/Addis_Ababa\",\"EAT-3\"],"
                              "[\"Africa/Algiers\",\"CET-1\"],"
                              "[\"Africa/Asmara\",\"EAT-3\"],"
                              "[\"Africa/Bamako\",\"GMT0\"],"
                              "[\"Africa/Bangui\",\"WAT-1\"],"
                              "[\"Africa/Banjul\",\"GMT0\"],"
                              "[\"Africa/Bissau\",\"GMT0\"],"
                              "[\"Africa/Blantyre\",\"CAT-2\"],"
                              "[\"Africa/Brazzaville\",\"WAT-1\"],"
                              "[\"Africa/Bujumbura\",\"CAT-2\"],"
                              "[\"Africa/Cairo\",\"EET-2\"],"
                              "[\"Africa/Casablanca\",\"<+01>-1\"],"
                              "[\"Africa/Ceuta\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Africa/Conakry\",\"GMT0\"],"
                              "[\"Africa/Dakar\",\"GMT0\"],"
                              "[\"Africa/Dar_es_Salaam\",\"EAT-3\"],"
                              "[\"Africa/Djibouti\",\"EAT-3\"],"
                              "[\"Africa/Douala\",\"WAT-1\"],"
                              "[\"Africa/El_Aaiun\",\"<+01>-1\"],"
                              "[\"Africa/Freetown\",\"GMT0\"],"
                              "[\"Africa/Gaborone\",\"CAT-2\"],"
                              "[\"Africa/Harare\",\"CAT-2\"],"
                              "[\"Africa/Johannesburg\",\"SAST-2\"],"
                              "[\"Africa/Juba\",\"CAT-2\"],"
                              "[\"Africa/Kampala\",\"EAT-3\"],"
                              "[\"Africa/Khartoum\",\"CAT-2\"],"
                              "[\"Africa/Kigali\",\"CAT-2\"],"
                              "[\"Africa/Kinshasa\",\"WAT-1\"],"
                              "[\"Africa/Lagos\",\"WAT-1\"],"
                              "[\"Africa/Libreville\",\"WAT-1\"],"
                              "[\"Africa/Lome\",\"GMT0\"],"
                              "[\"Africa/Luanda\",\"WAT-1\"],"
                              "[\"Africa/Lubumbashi\",\"CAT-2\"],"
                              "[\"Africa/Lusaka\",\"CAT-2\"],"
                              "[\"Africa/Malabo\",\"WAT-1\"],"
                              "[\"Africa/Maputo\",\"CAT-2\"],"
                              "[\"Africa/Maseru\",\"SAST-2\"],"
                              "[\"Africa/Mbabane\",\"SAST-2\"],"
                              "[\"Africa/Mogadishu\",\"EAT-3\"],"
                              "[\"Africa/Monrovia\",\"GMT0\"],"
                              "[\"Africa/Nairobi\",\"EAT-3\"],"
                              "[\"Africa/Ndjamena\",\"WAT-1\"],"
                              "[\"Africa/Niamey\",\"WAT-1\"],"
                              "[\"Africa/Nouakchott\",\"GMT0\"],"
                              "[\"Africa/Ouagadougou\",\"GMT0\"],"
                              "[\"Africa/Porto-Novo\",\"WAT-1\"],"
                              "[\"Africa/Sao_Tome\",\"GMT0\"],"
                              "[\"Africa/Tripoli\",\"EET-2\"],"
                              "[\"Africa/Tunis\",\"CET-1\"],"
                              "[\"Africa/Windhoek\",\"CAT-2\"],"
                              "[\"America/Adak\",\"HST10HDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Anchorage\",\"AKST9AKDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Anguilla\",\"AST4\"],"
                              "[\"America/Antigua\",\"AST4\"],"
                              "[\"America/Araguaina\",\"<-03>3\"],"
                              "[\"America/Argentina/Buenos_Aires\",\"<-03>3\"],"
                              "[\"America/Argentina/Catamarca\",\"<-03>3\"],"
                              "[\"America/Argentina/Cordoba\",\"<-03>3\"],"
                              "[\"America/Argentina/Jujuy\",\"<-03>3\"],"
                              "[\"America/Argentina/La_Rioja\",\"<-03>3\"],"
                              "[\"America/Argentina/Mendoza\",\"<-03>3\"],"
                              "[\"America/Argentina/Rio_Gallegos\",\"<-03>3\"],"
                              "[\"America/Argentina/Salta\",\"<-03>3\"],"
                              "[\"America/Argentina/San_Juan\",\"<-03>3\"],"
                              "[\"America/Argentina/San_Luis\",\"<-03>3\"],"
                              "[\"America/Argentina/Tucuman\",\"<-03>3\"],"
                              "[\"America/Argentina/Ushuaia\",\"<-03>3\"],"
                              "[\"America/Aruba\",\"AST4\"],"
                              "[\"America/Asuncion\",\"<-04>4<-03>,M10.1.0/0,M3.4.0/0\"],"
                              "[\"America/Atikokan\",\"EST5\"],"
                              "[\"America/Bahia\",\"<-03>3\"],"
                              "[\"America/Bahia_Banderas\",\"CST6\"],"
                              "[\"America/Barbados\",\"AST4\"],"
                              "[\"America/Belem\",\"<-03>3\"],"
                              "[\"America/Belize\",\"CST6\"],"
                              "[\"America/Blanc-Sablon\",\"AST4\"],"
                              "[\"America/Boa_Vista\",\"<-04>4\"],"
                              "[\"America/Bogota\",\"<-05>5\"],"
                              "[\"America/Boise\",\"MST7MDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Cambridge_Bay\",\"MST7MDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Campo_Grande\",\"<-04>4\"],"
                              "[\"America/Cancun\",\"EST5\"],"
                              "[\"America/Caracas\",\"<-04>4\"],"
                              "[\"America/Cayenne\",\"<-03>3\"],"
                              "[\"America/Cayman\",\"EST5\"],"
                              "[\"America/Chicago\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Chihuahua\",\"CST6\"],"
                              "[\"America/Costa_Rica\",\"CST6\"],"
                              "[\"America/Creston\",\"MST7\"],"
                              "[\"America/Cuiaba\",\"<-04>4\"],"
                              "[\"America/Curacao\",\"AST4\"],"
                              "[\"America/Danmarkshavn\",\"GMT0\"],"
                              "[\"America/Dawson\",\"MST7\"],"
                              "[\"America/Dawson_Creek\",\"MST7\"],"
                              "[\"America/Denver\",\"MST7MDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Detroit\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Dominica\",\"AST4\"],"
                              "[\"America/Edmonton\",\"MST7MDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Eirunepe\",\"<-05>5\"],"
                              "[\"America/El_Salvador\",\"CST6\"],"
                              "[\"America/Fortaleza\",\"<-03>3\"],"
                              "[\"America/Fort_Nelson\",\"MST7\"],"
                              "[\"America/Glace_Bay\",\"AST4ADT,M3.2.0,M11.1.0\"],"
                              "[\"America/Godthab\",\"<-02>2\"],"
                              "[\"America/Goose_Bay\",\"AST4ADT,M3.2.0,M11.1.0\"],"
                              "[\"America/Grand_Turk\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Grenada\",\"AST4\"],"
                              "[\"America/Guadeloupe\",\"AST4\"],"
                              "[\"America/Guatemala\",\"CST6\"],"
                              "[\"America/Guayaquil\",\"<-05>5\"],"
                              "[\"America/Guyana\",\"<-04>4\"],"
                              "[\"America/Halifax\",\"AST4ADT,M3.2.0,M11.1.0\"],"
                              "[\"America/Havana\",\"CST5CDT,M3.2.0/0,M11.1.0/1\"],"
                              "[\"America/Hermosillo\",\"MST7\"],"
                              "[\"America/Indiana/Indianapolis\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Indiana/Knox\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Indiana/Marengo\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Indiana/Petersburg\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Indiana/Tell_City\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Indiana/Vevay\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Indiana/Vincennes\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Indiana/Winamac\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Inuvik\",\"MST7MDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Iqaluit\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Jamaica\",\"EST5\"],"
                              "[\"America/Juneau\",\"AKST9AKDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Kentucky/Louisville\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Kentucky/Monticello\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Kralendijk\",\"AST4\"],"
                              "[\"America/La_Paz\",\"<-04>4\"],"
                              "[\"America/Lima\",\"<-05>5\"],"
                              "[\"America/Los_Angeles\",\"PST8PDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Lower_Princes\",\"AST4\"],"
                              "[\"America/Maceio\",\"<-03>3\"],"
                              "[\"America/Managua\",\"CST6\"],"
                              "[\"America/Manaus\",\"<-04>4\"],"
                              "[\"America/Marigot\",\"AST4\"],"
                              "[\"America/Martinique\",\"AST4\"],"
                              "[\"America/Matamoros\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Mazatlan\",\"MST7\"],"
                              "[\"America/Menominee\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Merida\",\"CST6\"],"
                              "[\"America/Metlakatla\",\"AKST9AKDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Mexico_City\",\"CST6\"],"
                              "[\"America/Miquelon\",\"<-03>3<-02>,M3.2.0,M11.1.0\"],"
                              "[\"America/Moncton\",\"AST4ADT,M3.2.0,M11.1.0\"],"
                              "[\"America/Monterrey\",\"CST6\"],"
                              "[\"America/Montevideo\",\"<-03>3\"],"
                              "[\"America/Montreal\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Montserrat\",\"AST4\"],"
                              "[\"America/Nassau\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/New_York\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Nipigon\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Nome\",\"AKST9AKDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Noronha\",\"<-02>2\"],"
                              "[\"America/North_Dakota/Beulah\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/North_Dakota/Center\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/North_Dakota/New_Salem\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Nuuk\",\"<-02>2\"],"
                              "[\"America/Ojinaga\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Panama\",\"EST5\"],"
                              "[\"America/Pangnirtung\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Paramaribo\",\"<-03>3\"],"
                              "[\"America/Phoenix\",\"MST7\"],"
                              "[\"America/Port-au-Prince\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Port_of_Spain\",\"AST4\"],"
                              "[\"America/Porto_Velho\",\"<-04>4\"],"
                              "[\"America/Puerto_Rico\",\"AST4\"],"
                              "[\"America/Punta_Arenas\",\"<-03>3\"],"
                              "[\"America/Rainy_River\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Rankin_Inlet\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Recife\",\"<-03>3\"],"
                              "[\"America/Regina\",\"CST6\"],"
                              "[\"America/Resolute\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Rio_Branco\",\"<-05>5\"],"
                              "[\"America/Santarem\",\"<-03>3\"],"
                              "[\"America/Santiago\",\"<-04>4<-03>,M9.1.6/24,M4.1.6/24\"],"
                              "[\"America/Santo_Domingo\",\"AST4\"],"
                              "[\"America/Sao_Paulo\",\"<-03>3\"],"
                              "[\"America/Scoresbysund\",\"<-01>1<+00>,M3.5.0/0,M10.5.0/1\"],"
                              "[\"America/Sitka\",\"AKST9AKDT,M3.2.0,M11.1.0\"],"
                              "[\"America/St_Barthelemy\",\"AST4\"],"
                              "[\"America/St_Johns\",\"NST3:30NDT,M3.2.0,M11.1.0\"],"
                              "[\"America/St_Kitts\",\"AST4\"],"
                              "[\"America/St_Lucia\",\"AST4\"],"
                              "[\"America/St_Thomas\",\"AST4\"],"
                              "[\"America/St_Vincent\",\"AST4\"],"
                              "[\"America/Swift_Current\",\"CST6\"],"
                              "[\"America/Tegucigalpa\",\"CST6\"],"
                              "[\"America/Thule\",\"AST4ADT,M3.2.0,M11.1.0\"],"
                              "[\"America/Thunder_Bay\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Tijuana\",\"PST8PDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Toronto\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Tortola\",\"AST4\"],"
                              "[\"America/Vancouver\",\"PST8PDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Whitehorse\",\"MST7\"],"
                              "[\"America/Winnipeg\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Yakutat\",\"AKST9AKDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Yellowknife\",\"MST7MDT,M3.2.0,M11.1.0\"],"
                              "[\"Antarctica/Casey\",\"<+11>-11\"],"
                              "[\"Antarctica/Davis\",\"<+07>-7\"],"
                              "[\"Antarctica/DumontDUrville\",\"<+10>-10\"],"
                              "[\"Antarctica/Macquarie\",\"AEST-10AEDT,M10.1.0,M4.1.0/3\"],"
                              "[\"Antarctica/Mawson\",\"<+05>-5\"],"
                              "[\"Antarctica/McMurdo\",\"NZST-12NZDT,M9.5.0,M4.1.0/3\"],"
                              "[\"Antarctica/Palmer\",\"<-03>3\"],"
                              "[\"Antarctica/Rothera\",\"<-03>3\"],"
                              "[\"Antarctica/Syowa\",\"<+03>-3\"],"
                              "[\"Antarctica/Troll\",\"<+00>0<+02>-2,M3.5.0/1,M10.5.0/3\"],"
                              "[\"Antarctica/Vostok\",\"<+06>-6\"],"
                              "[\"Arctic/Longyearbyen\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Asia/Aden\",\"<+03>-3\"],"
                              "[\"Asia/Almaty\",\"<+06>-6\"],"
                              "[\"Asia/Amman\",\"<+03>-3\"],"
                              "[\"Asia/Anadyr\",\"<+12>-12\"],"
                              "[\"Asia/Aqtau\",\"<+05>-5\"],"
                              "[\"Asia/Aqtobe\",\"<+05>-5\"],"
                              "[\"Asia/Ashgabat\",\"<+05>-5\"],"
                              "[\"Asia/Atyrau\",\"<+05>-5\"],"
                              "[\"Asia/Baghdad\",\"<+03>-3\"],"
                              "[\"Asia/Bahrain\",\"<+03>-3\"],"
                              "[\"Asia/Baku\",\"<+04>-4\"],"
                              "[\"Asia/Bangkok\",\"<+07>-7\"],"
                              "[\"Asia/Barnaul\",\"<+07>-7\"],"
                              "[\"Asia/Beirut\",\"EET-2EEST,M3.5.0/0,M10.5.0/0\"],"
                              "[\"Asia/Bishkek\",\"<+06>-6\"],"
                              "[\"Asia/Brunei\",\"<+08>-8\"],"
                              "[\"Asia/Chita\",\"<+09>-9\"],"
                              "[\"Asia/Choibalsan\",\"<+08>-8\"],"
                              "[\"Asia/Colombo\",\"<+0530>-5:30\"],"
                              "[\"Asia/Damascus\",\"<+03>-3\"],"
                              "[\"Asia/Dhaka\",\"<+06>-6\"],"
                              "[\"Asia/Dili\",\"<+09>-9\"],"
                              "[\"Asia/Dubai\",\"<+04>-4\"],"
                              "[\"Asia/Dushanbe\",\"<+05>-5\"],"
                              "[\"Asia/Famagusta\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Asia/Gaza\",\"EET-2EEST,M3.4.4/50,M10.4.4/50\"],"
                              "[\"Asia/Hebron\",\"EET-2EEST,M3.4.4/50,M10.4.4/50\"],"
                              "[\"Asia/Ho_Chi_Minh\",\"<+07>-7\"],"
                              "[\"Asia/Hong_Kong\",\"HKT-8\"],"
                              "[\"Asia/Hovd\",\"<+07>-7\"],"
                              "[\"Asia/Irkutsk\",\"<+08>-8\"],"
                              "[\"Asia/Jakarta\",\"WIB-7\"],"
                              "[\"Asia/Jayapura\",\"WIT-9\"],"
                              "[\"Asia/Jerusalem\",\"IST-2IDT,M3.4.4/26,M10.5.0\"],"
                              "[\"Asia/Kabul\",\"<+0430>-4:30\"],"
                              "[\"Asia/Kamchatka\",\"<+12>-12\"],"
                              "[\"Asia/Karachi\",\"PKT-5\"],"
                              "[\"Asia/Kathmandu\",\"<+0545>-5:45\"],"
                              "[\"Asia/Khandyga\",\"<+09>-9\"],"
                              "[\"Asia/Kolkata\",\"IST-5:30\"],"
                              "[\"Asia/Krasnoyarsk\",\"<+07>-7\"],"
                              "[\"Asia/Kuala_Lumpur\",\"<+08>-8\"],"
                              "[\"Asia/Kuching\",\"<+08>-8\"],"
                              "[\"Asia/Kuwait\",\"<+03>-3\"],"
                              "[\"Asia/Macau\",\"CST-8\"],"
                              "[\"Asia/Magadan\",\"<+11>-11\"],"
                              "[\"Asia/Makassar\",\"WITA-8\"],"
                              "[\"Asia/Manila\",\"PST-8\"],"
                              "[\"Asia/Muscat\",\"<+04>-4\"],"
                              "[\"Asia/Nicosia\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Asia/Novokuznetsk\",\"<+07>-7\"],"
                              "[\"Asia/Novosibirsk\",\"<+07>-7\"],"
                              "[\"Asia/Omsk\",\"<+06>-6\"],"
                              "[\"Asia/Oral\",\"<+05>-5\"],"
                              "[\"Asia/Phnom_Penh\",\"<+07>-7\"],"
                              "[\"Asia/Pontianak\",\"WIB-7\"],"
                              "[\"Asia/Pyongyang\",\"KST-9\"],"
                              "[\"Asia/Qatar\",\"<+03>-3\"],"
                              "[\"Asia/Qyzylorda\",\"<+05>-5\"],"
                              "[\"Asia/Riyadh\",\"<+03>-3\"],"
                              "[\"Asia/Sakhalin\",\"<+11>-11\"],"
                              "[\"Asia/Samarkand\",\"<+05>-5\"],"
                              "[\"Asia/Seoul\",\"KST-9\"],"
                              "[\"Asia/Shanghai\",\"CST-8\"],"
                              "[\"Asia/Singapore\",\"<+08>-8\"],"
                              "[\"Asia/Srednekolymsk\",\"<+11>-11\"],"
                              "[\"Asia/Taipei\",\"CST-8\"],"
                              "[\"Asia/Tashkent\",\"<+05>-5\"],"
                              "[\"Asia/Tbilisi\",\"<+04>-4\"],"
                              "[\"Asia/Tehran\",\"<+0330>-3:30\"],"
                              "[\"Asia/Thimphu\",\"<+06>-6\"],"
                              "[\"Asia/Tokyo\",\"JST-9\"],"
                              "[\"Asia/Tomsk\",\"<+07>-7\"],"
                              "[\"Asia/Ulaanbaatar\",\"<+08>-8\"],"
                              "[\"Asia/Urumqi\",\"<+06>-6\"],"
                              "[\"Asia/Ust-Nera\",\"<+10>-10\"],"
                              "[\"Asia/Vientiane\",\"<+07>-7\"],"
                              "[\"Asia/Vladivostok\",\"<+10>-10\"],"
                              "[\"Asia/Yakutsk\",\"<+09>-9\"],"
                              "[\"Asia/Yangon\",\"<+0630>-6:30\"],"
                              "[\"Asia/Yekaterinburg\",\"<+05>-5\"],"
                              "[\"Asia/Yerevan\",\"<+04>-4\"],"
                              "[\"Atlantic/Azores\",\"<-01>1<+00>,M3.5.0/0,M10.5.0/1\"],"
                              "[\"Atlantic/Bermuda\",\"AST4ADT,M3.2.0,M11.1.0\"],"
                              "[\"Atlantic/Canary\",\"WET0WEST,M3.5.0/1,M10.5.0\"],"
                              "[\"Atlantic/Cape_Verde\",\"<-01>1\"],"
                              "[\"Atlantic/Faroe\",\"WET0WEST,M3.5.0/1,M10.5.0\"],"
                              "[\"Atlantic/Madeira\",\"WET0WEST,M3.5.0/1,M10.5.0\"],"
                              "[\"Atlantic/Reykjavik\",\"GMT0\"],"
                              "[\"Atlantic/South_Georgia\",\"<-02>2\"],"
                              "[\"Atlantic/Stanley\",\"<-03>3\"],"
                              "[\"Atlantic/St_Helena\",\"GMT0\"],"
                              "[\"Australia/Adelaide\",\"ACST-9:30ACDT,M10.1.0,M4.1.0/3\"],"
                              "[\"Australia/Brisbane\",\"AEST-10\"],"
                              "[\"Australia/Broken_Hill\",\"ACST-9:30ACDT,M10.1.0,M4.1.0/3\"],"
                              "[\"Australia/Currie\",\"AEST-10AEDT,M10.1.0,M4.1.0/3\"],"
                              "[\"Australia/Darwin\",\"ACST-9:30\"],"
                              "[\"Australia/Eucla\",\"<+0845>-8:45\"],"
                              "[\"Australia/Hobart\",\"AEST-10AEDT,M10.1.0,M4.1.0/3\"],"
                              "[\"Australia/Lindeman\",\"AEST-10\"],"
                              "[\"Australia/Lord_Howe\",\"<+1030>-10:30<+11>-11,M10.1.0,M4.1.0\"],"
                              "[\"Australia/Melbourne\",\"AEST-10AEDT,M10.1.0,M4.1.0/3\"],"
                              "[\"Australia/Perth\",\"AWST-8\"],"
                              "[\"Australia/Sydney\",\"AEST-10AEDT,M10.1.0,M4.1.0/3\"],"
                              "[\"Europe/Amsterdam\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Andorra\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Astrakhan\",\"<+04>-4\"],"
                              "[\"Europe/Athens\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Belgrade\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Berlin\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Bratislava\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Brussels\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Bucharest\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Budapest\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Busingen\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Chisinau\",\"EET-2EEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Copenhagen\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Dublin\",\"IST-1GMT0,M10.5.0,M3.5.0/1\"],"
                              "[\"Europe/Gibraltar\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Guernsey\",\"GMT0BST,M3.5.0/1,M10.5.0\"],"
                              "[\"Europe/Helsinki\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Isle_of_Man\",\"GMT0BST,M3.5.0/1,M10.5.0\"],"
                              "[\"Europe/Istanbul\",\"<+03>-3\"],"
                              "[\"Europe/Jersey\",\"GMT0BST,M3.5.0/1,M10.5.0\"],"
                              "[\"Europe/Kaliningrad\",\"EET-2\"],"
                              "[\"Europe/Kiev\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Kirov\",\"<+03>-3\"],"
                              "[\"Europe/Lisbon\",\"WET0WEST,M3.5.0/1,M10.5.0\"],"
                              "[\"Europe/Ljubljana\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/London\",\"GMT0BST,M3.5.0/1,M10.5.0\"],"
                              "[\"Europe/Luxembourg\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Madrid\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Malta\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Mariehamn\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Minsk\",\"<+03>-3\"],"
                              "[\"Europe/Monaco\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Moscow\",\"MSK-3\"],"
                              "[\"Europe/Oslo\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Paris\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Podgorica\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Prague\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Riga\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Rome\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Samara\",\"<+04>-4\"],"
                              "[\"Europe/San_Marino\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Sarajevo\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Saratov\",\"<+04>-4\"],"
                              "[\"Europe/Simferopol\",\"MSK-3\"],"
                              "[\"Europe/Skopje\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Sofia\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Stockholm\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Tallinn\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Tirane\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Ulyanovsk\",\"<+04>-4\"],"
                              "[\"Europe/Uzhgorod\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Vaduz\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Vatican\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Vienna\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Vilnius\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Volgograd\",\"<+03>-3\"],"
                              "[\"Europe/Warsaw\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Zagreb\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Zaporozhye\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Zurich\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Indian/Antananarivo\",\"EAT-3\"],"
                              "[\"Indian/Chagos\",\"<+06>-6\"],"
                              "[\"Indian/Christmas\",\"<+07>-7\"],"
                              "[\"Indian/Cocos\",\"<+0630>-6:30\"],"
                              "[\"Indian/Comoro\",\"EAT-3\"],"
                              "[\"Indian/Kerguelen\",\"<+05>-5\"],"
                              "[\"Indian/Mahe\",\"<+04>-4\"],"
                              "[\"Indian/Maldives\",\"<+05>-5\"],"
                              "[\"Indian/Mauritius\",\"<+04>-4\"],"
                              "[\"Indian/Mayotte\",\"EAT-3\"],"
                              "[\"Indian/Reunion\",\"<+04>-4\"],"
                              "[\"Pacific/Apia\",\"<+13>-13\"],"
                              "[\"Pacific/Auckland\",\"NZST-12NZDT,M9.5.0,M4.1.0/3\"],"
                              "[\"Pacific/Bougainville\",\"<+11>-11\"],"
                              "[\"Pacific/Chatham\",\"<+1245>-12:45<+1345>,M9.5.0/2:45,M4.1.0/3:45\"],"
                              "[\"Pacific/Chuuk\",\"<+10>-10\"],"
                              "[\"Pacific/Easter\",\"<-06>6<-05>,M9.1.6/22,M4.1.6/22\"],"
                              "[\"Pacific/Efate\",\"<+11>-11\"],"
                              "[\"Pacific/Enderbury\",\"<+13>-13\"],"
                              "[\"Pacific/Fakaofo\",\"<+13>-13\"],"
                              "[\"Pacific/Fiji\",\"<+12>-12\"],"
                              "[\"Pacific/Funafuti\",\"<+12>-12\"],"
                              "[\"Pacific/Galapagos\",\"<-06>6\"],"
                              "[\"Pacific/Gambier\",\"<-09>9\"],"
                              "[\"Pacific/Guadalcanal\",\"<+11>-11\"],"
                              "[\"Pacific/Guam\",\"ChST-10\"],"
                              "[\"Pacific/Honolulu\",\"HST10\"],"
                              "[\"Pacific/Kiritimati\",\"<+14>-14\"],"
                              "[\"Pacific/Kosrae\",\"<+11>-11\"],"
                              "[\"Pacific/Kwajalein\",\"<+12>-12\"],"
                              "[\"Pacific/Majuro\",\"<+12>-12\"],"
                              "[\"Pacific/Marquesas\",\"<-0930>9:30\"],"
                              "[\"Pacific/Midway\",\"SST11\"],"
                              "[\"Pacific/Nauru\",\"<+12>-12\"],"
                              "[\"Pacific/Niue\",\"<-11>11\"],"
                              "[\"Pacific/Norfolk\",\"<+11>-11<+12>,M10.1.0,M4.1.0/3\"],"
                              "[\"Pacific/Noumea\",\"<+11>-11\"],"
                              "[\"Pacific/Pago_Pago\",\"SST11\"],"
                              "[\"Pacific/Palau\",\"<+09>-9\"],"
                              "[\"Pacific/Pitcairn\",\"<-08>8\"],"
                              "[\"Pacific/Pohnpei\",\"<+11>-11\"],"
                              "[\"Pacific/Port_Moresby\",\"<+10>-10\"],"
                              "[\"Pacific/Rarotonga\",\"<-10>10\"],"
                              "[\"Pacific/Saipan\",\"ChST-10\"],"
                              "[\"Pacific/Tahiti\",\"<-10>10\"],"
                              "[\"Pacific/Tarawa\",\"<+12>-12\"],"
                              "[\"Pacific/Tongatapu\",\"<+13>-13\"],"
                              "[\"Pacific/Wake\",\"<+12>-12\"],"
                              "[\"Pacific/Wallis\",\"<+12>-12\"],"
                              "[\"Etc/GMT\",\"GMT0\"],"
                              "[\"Etc/GMT-0\",\"GMT0\"],"
                              "[\"Etc/GMT-1\",\"<+01>-1\"],"
                              "[\"Etc/GMT-2\",\"<+02>-2\"],"
                              "[\"Etc/GMT-3\",\"<+03>-3\"],"
                              "[\"Etc/GMT-4\",\"<+04>-4\"],"
                              "[\"Etc/GMT-5\",\"<+05>-5\"],"
                              "[\"Etc/GMT-6\",\"<+06>-6\"],"
                              "[\"Etc/GMT-7\",\"<+07>-7\"],"
                              "[\"Etc/GMT-8\",\"<+08>-8\"],"
                              "[\"Etc/GMT-9\",\"<+09>-9\"],"
                              "[\"Etc/GMT-10\",\"<+10>-10\"],"
                              "[\"Etc/GMT-11\",\"<+11>-11\"],"
                              "[\"Etc/GMT-12\",\"<+12>-12\"],"
                              "[\"Etc/GMT-13\",\"<+13>-13\"],"
                              "[\"Etc/GMT-14\",\"<+14>-14\"],"
                              "[\"Etc/GMT0\",\"GMT0\"],"
                              "[\"Etc/GMT+0\",\"GMT0\"],"
                              "[\"Etc/GMT+1\",\"<-01>1\"],"
                              "[\"Etc/GMT+2\",\"<-02>2\"],"
                              "[\"Etc/GMT+3\",\"<-03>3\"],"
                              "[\"Etc/GMT+4\",\"<-04>4\"],"
                              "[\"Etc/GMT+5\",\"<-05>5\"],"
                              "[\"Etc/GMT+6\",\"<-06>6\"],"
                              "[\"Etc/GMT+7\",\"<-07>7\"],"
                              "[\"Etc/GMT+8\",\"<-08>8\"],"
                              "[\"Etc/GMT+9\",\"<-09>9\"],"
                              "[\"Etc/GMT+10\",\"<-10>10\"],"
                              "[\"Etc/GMT+11\",\"<-11>11\"],"
                              "[\"Etc/GMT+12\",\"<-12>12\"],"
                              "[\"Etc/UCT\",\"UTC0\"],"
                              "[\"Etc/UTC\",\"UTC0\"],"
                              "[\"Etc/Greenwich\",\"GMT0\"],"
                              "[\"Etc/Universal\",\"UTC0\"],"
                              "[\"Etc/Zulu\",\"UTC0\"]]";
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char locations_json[] = "[[ \"Africa/Accra\", \"Accra\", \"5.55\", \"-0.22\" ],"
                              "[ \"Africa/Addis_Ababa\", \"Addis Ababa\", \"9.03\", \"38.70\" ],"
                              "[ \"Africa/Algiers\", \"Algiers\", \"36.78\", \"3.05\" ],"
                              "[ \"Africa/Asmara\", \"Asmara\", \"15.33\", \"38.88\" ],"
                              "[ \"Africa/Bamako\", \"Bamako\", \"12.65\", \"-8.00\" ],"
                              "[ \"Africa/Bangui\", \"Bangui\", \"4.37\", \"18.58\" ],"
                              "[ \"Africa/Banjul\", \"Banjul\", \"13.47\", \"-16.65\" ],"
                              "[ \"Africa/Bissau\", \"Bissau\", \"11.85\", \"-15.58\" ],"
                              "[ \"Africa/Blantyre\", \"Blantyre\", \"-15.78\", \"35.00\" ],"
                              "[ \"Africa/Brazzaville\", \"Brazzaville\", \"-4.27\", \"15.28\" ],"
                              "[ \"Africa/Bujumbura\", \"Bujumbura\", \"-3.38\", \"29.37\" ],"
                              "[ \"Africa/Cairo\", \"Cairo\", \"30.04\", \"31.24\" ],"
                              "[ \"Africa/Casablanca\", \"Casablanca\", \"33.65\", \"-7.58\" ],"
                              "[ \"Africa/Ceuta\", \"Ceuta\", \"35.88\", \"-5.32\" ],"
                              "[ \"Africa/Conakry\", \"Conakry\", \"9.52\", \"-13.72\" ],"
                              "[ \"Africa/Dakar\", \"Dakar\", \"14.67\", \"-17.43\" ],"
                              "[ \"Africa/Dar_es_Salaam\", \"Dar es Salaam\", \"-6.80\", \"39.28\" ],"
                              "[ \"Africa/Djibouti\", \"Djibouti\", \"11.60\", \"43.15\" ],"
                              "[ \"Africa/Douala\", \"Douala\", \"4.05\", \"9.70\" ],"
                              "[ \"Africa/El_Aaiun\", \"El Aaiun\", \"27.15\", \"-13.20\" ],"
                              "[ \"Africa/Freetown\", \"Freetown\", \"8.50\", \"-13.25\" ],"
                              "[ \"Africa/Gaborone\", \"Gaborone\", \"-24.65\", \"25.92\" ],"
                              "[ \"Africa/Harare\", \"Harare\", \"-17.83\", \"31.05\" ],"
                              "[ \"Africa/Johannesburg\", \"Johannesburg\", \"-26.20\", \"28.05\" ],"
                              "[ \"Africa/Juba\", \"Juba\", \"4.85\", \"31.62\" ],"
                              "[ \"Africa/Kampala\", \"Kampala\", \"0.32\", \"32.42\" ],"
                              "[ \"Africa/Khartoum\", \"Khartoum\", \"15.60\", \"32.53\" ],"
                              "[ \"Africa/Kigali\", \"Kigali\", \"-1.95\", \"30.07\" ],"
                              "[ \"Africa/Kinshasa\", \"Kinshasa\", \"-4.30\", \"15.30\" ],"
                              "[ \"Africa/Lagos\", \"Lagos\", \"6.45\", \"3.40\" ],"
                              "[ \"Africa/Libreville\", \"Libreville\", \"0.38\", \"9.45\" ],"
                              "[ \"Africa/Lome\", \"Lome\", \"6.13\", \"1.22\" ],"
                              "[ \"Africa/Luanda\", \"Luanda\", \"-8.80\", \"13.23\" ],"
                              "[ \"Africa/Lubumbashi\", \"Lubumbashi\", \"-11.67\", \"27.47\" ],"
                              "[ \"Africa/Lusaka\", \"Lusaka\", \"-15.42\", \"28.28\" ],"
                              "[ \"Africa/Malabo\", \"Malabo\", \"3.75\", \"8.78\" ],"
                              "[ \"Africa/Maputo\", \"Maputo\", \"-25.97\", \"32.58\" ],"
                              "[ \"Africa/Maseru\", \"Maseru\", \"-29.47\", \"27.50\" ],"
                              "[ \"Africa/Mbabane\", \"Mbabane\", \"-26.30\", \"31.10\" ],"
                              "[ \"Africa/Mogadishu\", \"Mogadishu\", \"2.07\", \"45.37\" ],"
                              "[ \"Africa/Monrovia\", \"Monrovia\", \"6.30\", \"-10.78\" ],"
                              "[ \"Africa/Nairobi\", \"Nairobi\", \"-1.29\", \"36.82\" ],"
                              "[ \"Africa/Ndjamena\", \"Ndjamena\", \"12.12\", \"15.05\" ],"
                              "[ \"Africa/Niamey\", \"Niamey\", \"13.52\", \"2.12\" ],"
                              "[ \"Africa/Nouakchott\", \"Nouakchott\", \"18.10\", \"-15.95\" ],"
                              "[ \"Africa/Ouagadougou\", \"Ouagadougou\", \"12.37\", \"-1.52\" ],"
                              "[ \"Africa/Porto-Novo\", \"Porto-Novo\", \"6.48\", \"2.62\" ],"
                              "[ \"Africa/Sao_Tome\", \"São Tomé\", \"0.33\", \"6.73\" ],"
                              "[ \"Africa/Tripoli\", \"Tripoli\", \"32.90\", \"13.18\" ],"
                              "[ \"Africa/Tunis\", \"Tunis\", \"36.80\", \"10.18\" ],"
                              "[ \"Africa/Windhoek\", \"Windhoek\", \"-22.57\", \"17.10\" ],"
                              "[ \"America/Adak\", \"Adak\", \"51.88\", \"-176.66\" ],"
                              "[ \"America/Anchorage\", \"Anchorage\", \"61.22\", \"-149.90\" ],"
                              "[ \"America/Anguilla\", \"Anguilla\", \"18.20\", \"-63.07\" ],"
                              "[ \"America/Antigua\", \"Antigua\", \"17.05\", \"-61.80\" ],"
                              "[ \"America/Araguaina\", \"Araguaina\", \"-7.20\", \"-48.20\" ],"
                              "[ \"America/Argentina/Buenos_Aires\", \"Buenos Aires\", \"-34.60\", \"-58.45\" ],"
                              "[ \"America/Argentina/Catamarca\", \"Catamarca\", \"-28.47\", \"-65.78\" ],"
                              "[ \"America/Argentina/Cordoba\", \"Cordoba\", \"-31.40\", \"-64.18\" ],"
                              "[ \"America/Argentina/Jujuy\", \"Jujuy\", \"-24.18\", \"-65.30\" ],"
                              "[ \"America/Argentina/La_Rioja\", \"La Rioja\", \"-29.43\", \"-66.85\" ],"
                              "[ \"America/Argentina/Mendoza\", \"Mendoza\", \"-32.88\", \"-68.82\" ],"
                              "[ \"America/Argentina/Rio_Gallegos\", \"Rio Gallegos\", \"-51.63\", \"-69.22\" ],"
                              "[ \"America/Argentina/Salta\", \"Salta\", \"-24.78\", \"-65.42\" ],"
                              "[ \"America/Argentina/San_Juan\", \"San Juan\", \"-31.53\", \"-68.52\" ],"
                              "[ \"America/Argentina/San_Luis\", \"San Luis\", \"-33.32\", \"-66.35\" ],"
                              "[ \"America/Argentina/Tucuman\", \"Tucuman\", \"-26.82\", \"-65.22\" ],"
                              "[ \"America/Argentina/Ushuaia\", \"Ushuaia\", \"-54.80\", \"-68.30\" ],"
                              "[ \"America/Aruba\", \"Aruba\", \"12.50\", \"-69.97\" ],"
                              "[ \"America/Asuncion\", \"Asuncion\", \"-25.27\", \"-57.67\" ],"
                              "[ \"America/Atikokan\", \"Atikokan\", \"48.76\", \"-91.62\" ],"
                              "[ \"America/Bahia\", \"Bahia\", \"-12.98\", \"-38.52\" ],"
                              "[ \"America/Bahia_Banderas\", \"Bahia Banderas\", \"20.80\", \"-105.25\" ],"
                              "[ \"America/Barbados\", \"Barbados\", \"13.10\", \"-59.62\" ],"
                              "[ \"America/Belem\", \"Belem\", \"-1.45\", \"-48.48\" ],"
                              "[ \"America/Belize\", \"Belize\", \"17.50\", \"-88.20\" ],"
                              "[ \"America/Blanc-Sablon\", \"Blanc-Sablon\", \"51.42\", \"-57.12\" ],"
                              "[ \"America/Boa_Vista\", \"Boa Vista\", \"2.82\", \"-60.67\" ],"
                              "[ \"America/Bogota\", \"Bogota\", \"4.60\", \"-74.08\" ],"
                              "[ \"America/Boise\", \"Boise\", \"43.61\", \"-116.20\" ],"
                              "[ \"America/Cambridge_Bay\", \"Cambridge Bay\", \"69.11\", \"-105.05\" ],"
                              "[ \"America/Campo_Grande\", \"Campo Grande\", \"-20.45\", \"-54.62\" ],"
                              "[ \"America/Cancun\", \"Cancun\", \"21.08\", \"-86.77\" ],"
                              "[ \"America/Caracas\", \"Caracas\", \"10.50\", \"-66.93\" ],"
                              "[ \"America/Cayenne\", \"Cayenne\", \"4.93\", \"-52.33\" ],"
                              "[ \"America/Cayman\", \"Cayman\", \"19.30\", \"-81.38\" ],"
                              "[ \"America/Chicago\", \"Chicago\", \"41.88\", \"-87.63\" ],"
                              "[ \"America/Chihuahua\", \"Chihuahua\", \"28.63\", \"-106.08\" ],"
                              "[ \"America/Costa_Rica\", \"Costa Rica\", \"9.93\", \"-84.08\" ],"
                              "[ \"America/Creston\", \"Creston\", \"49.10\", \"-116.52\" ],"
                              "[ \"America/Cuiaba\", \"Cuiaba\", \"-15.58\", \"-56.08\" ],"
                              "[ \"America/Curacao\", \"Curacao\", \"12.18\", \"-69.00\" ],"
                              "[ \"America/Danmarkshavn\", \"Danmarkshavn\", \"76.77\", \"-18.67\" ],"
                              "[ \"America/Dawson\", \"Dawson\", \"64.07\", \"-139.42\" ],"
                              "[ \"America/Dawson_Creek\", \"Dawson Creek\", \"55.77\", \"-120.23\" ],"
                              "[ \"America/Denver\", \"Denver\", \"39.74\", \"-104.99\" ],"
                              "[ \"America/Detroit\", \"Detroit\", \"42.33\", \"-83.05\" ],"
                              "[ \"America/Dominica\", \"Dominica\", \"15.30\", \"-61.40\" ],"
                              "[ \"America/Edmonton\", \"Edmonton\", \"53.55\", \"-113.47\" ],"
                              "[ \"America/Eirunepe\", \"Eirunepe\", \"-6.67\", \"-69.87\" ],"
                              "[ \"America/El_Salvador\", \"El Salvador\", \"13.70\", \"-89.20\" ],"
                              "[ \"America/Fortaleza\", \"Fortaleza\", \"-3.72\", \"-38.50\" ],"
                              "[ \"America/Fort_Nelson\", \"Fort Nelson\", \"58.80\", \"-122.70\" ],"
                              "[ \"America/Glace_Bay\", \"Glace Bay\", \"46.20\", \"-59.95\" ],"
                              "[ \"America/Godthab\", \"Nuuk\", \"64.18\", \"-51.73\" ],"
                              "[ \"America/Goose_Bay\", \"Goose Bay\", \"53.33\", \"-60.42\" ],"
                              "[ \"America/Grand_Turk\", \"Grand Turk\", \"21.47\", \"-71.13\" ],"
                              "[ \"America/Grenada\", \"Grenada\", \"12.05\", \"-61.75\" ],"
                              "[ \"America/Guadeloupe\", \"Guadeloupe\", \"16.23\", \"-61.53\" ],"
                              "[ \"America/Guatemala\", \"Guatemala\", \"14.63\", \"-90.52\" ],"
                              "[ \"America/Guayaquil\", \"Guayaquil\", \"-2.17\", \"-79.83\" ],"
                              "[ \"America/Guyana\", \"Guyana\", \"6.80\", \"-58.17\" ],"
                              "[ \"America/Halifax\", \"Halifax\", \"44.65\", \"-63.60\" ],"
                              "[ \"America/Havana\", \"Havana\", \"23.13\", \"-82.37\" ],"
                              "[ \"America/Hermosillo\", \"Hermosillo\", \"29.07\", \"-110.97\" ],"
                              "[ \"America/Indiana/Indianapolis\", \"Indianapolis\", \"39.77\", \"-86.16\" ],"
                              "[ \"America/Indiana/Knox\", \"Knox\", \"41.30\", \"-86.62\" ],"
                              "[ \"America/Indiana/Marengo\", \"Marengo\", \"38.38\", \"-86.34\" ],"
                              "[ \"America/Indiana/Petersburg\", \"Petersburg\", \"38.49\", \"-87.28\" ],"
                              "[ \"America/Indiana/Tell_City\", \"Tell City\", \"37.95\", \"-86.76\" ],"
                              "[ \"America/Indiana/Vevay\", \"Vevay\", \"38.75\", \"-85.07\" ],"
                              "[ \"America/Indiana/Vincennes\", \"Vincennes\", \"38.68\", \"-87.53\" ],"
                              "[ \"America/Indiana/Winamac\", \"Winamac\", \"41.05\", \"-86.60\" ],"
                              "[ \"America/Inuvik\", \"Inuvik\", \"68.35\", \"-133.72\" ],"
                              "[ \"America/Iqaluit\", \"Iqaluit\", \"63.73\", \"-68.47\" ],"
                              "[ \"America/Jamaica\", \"Jamaica\", \"17.97\", \"-76.79\" ],"
                              "[ \"America/Juneau\", \"Juneau\", \"58.30\", \"-134.42\" ],"
                              "[ \"America/Kentucky/Louisville\", \"Louisville\", \"38.25\", \"-85.76\" ],"
                              "[ \"America/Kentucky/Monticello\", \"Monticello\", \"36.83\", \"-84.85\" ],"
                              "[ \"America/Kralendijk\", \"Kralendijk\", \"12.15\", \"-68.28\" ],"
                              "[ \"America/La_Paz\", \"La Paz\", \"-16.50\", \"-68.15\" ],"
                              "[ \"America/Lima\", \"Lima\", \"-12.05\", \"-77.05\" ],"
                              "[ \"America/Los_Angeles\", \"Los Angeles\", \"34.05\", \"-118.24\" ],"
                              "[ \"America/Lower_Princes\", \"Lower Prince’s Quarter\", \"18.05\", \"-63.05\" ],"
                              "[ \"America/Maceio\", \"Maceio\", \"-9.67\", \"-35.72\" ],"
                              "[ \"America/Managua\", \"Managua\", \"12.15\", \"-86.28\" ],"
                              "[ \"America/Manaus\", \"Manaus\", \"-3.13\", \"-60.02\" ],"
                              "[ \"America/Marigot\", \"Marigot\", \"18.07\", \"-63.08\" ],"
                              "[ \"America/Martinique\", \"Martinique\", \"14.60\", \"-61.08\" ],"
                              "[ \"America/Matamoros\", \"Matamoros\", \"25.83\", \"-97.50\" ],"
                              "[ \"America/Mazatlan\", \"Mazatlan\", \"23.22\", \"-106.42\" ],"
                              "[ \"America/Menominee\", \"Menominee\", \"45.11\", \"-87.61\" ],"
                              "[ \"America/Merida\", \"Merida\", \"20.97\", \"-89.62\" ],"
                              "[ \"America/Metlakatla\", \"Metlakatla\", \"55.13\", \"-131.58\" ],"
                              "[ \"America/Mexico_City\", \"Mexico City\", \"19.40\", \"-99.15\" ],"
                              "[ \"America/Miquelon\", \"Miquelon\", \"47.05\", \"-56.33\" ],"
                              "[ \"America/Moncton\", \"Moncton\", \"46.10\", \"-64.78\" ],"
                              "[ \"America/Monterrey\", \"Monterrey\", \"25.67\", \"-100.32\" ],"
                              "[ \"America/Montevideo\", \"Montevideo\", \"-34.91\", \"-56.21\" ],"
                              "[ \"America/Montreal\", \"Montreal\", \"43.65\", \"-79.38\" ],"
                              "[ \"America/Montserrat\", \"Montserrat\", \"16.72\", \"-62.22\" ],"
                              "[ \"America/Nassau\", \"Nassau\", \"25.08\", \"-77.35\" ],"
                              "[ \"America/New_York\", \"New York\", \"40.71\", \"-74.01\" ],"
                              "[ \"America/Nipigon\", \"Nipigon\", \"43.65\", \"-79.38\" ],"
                              "[ \"America/Nome\", \"Nome\", \"64.50\", \"-165.41\" ],"
                              "[ \"America/Noronha\", \"Noronha\", \"-3.85\", \"-32.42\" ],"
                              "[ \"America/North_Dakota/Beulah\", \"Beulah\", \"47.26\", \"-101.78\" ],"
                              "[ \"America/North_Dakota/Center\", \"Center\", \"47.12\", \"-101.30\" ],"
                              "[ \"America/North_Dakota/New_Salem\", \"New Salem\", \"46.84\", \"-101.41\" ],"
                              "[ \"America/Nuuk\", \"Nuuk\", \"64.18\", \"-51.73\" ],"
                              "[ \"America/Ojinaga\", \"Ojinaga\", \"29.57\", \"-104.42\" ],"
                              "[ \"America/Panama\", \"Panama\", \"8.97\", \"-79.53\" ],"
                              "[ \"America/Pangnirtung\", \"Pangnirtung\", \"63.73\", \"-68.47\" ],"
                              "[ \"America/Paramaribo\", \"Paramaribo\", \"5.83\", \"-55.17\" ],"
                              "[ \"America/Phoenix\", \"Phoenix\", \"33.45\", \"-112.07\" ],"
                              "[ \"America/Port-au-Prince\", \"Port-au-Prince\", \"18.53\", \"-72.33\" ],"
                              "[ \"America/Port_of_Spain\", \"Port of Spain\", \"10.65\", \"-61.52\" ],"
                              "[ \"America/Porto_Velho\", \"Porto Velho\", \"-8.77\", \"-63.90\" ],"
                              "[ \"America/Puerto_Rico\", \"Puerto Rico\", \"18.47\", \"-66.11\" ],"
                              "[ \"America/Punta_Arenas\", \"Punta Arenas\", \"-53.15\", \"-70.92\" ],"
                              "[ \"America/Rainy_River\", \"Rainy River\", \"49.88\", \"-97.15\" ],"
                              "[ \"America/Rankin_Inlet\", \"Rankin Inlet\", \"62.82\", \"-92.08\" ],"
                              "[ \"America/Recife\", \"Recife\", \"-8.05\", \"-34.90\" ],"
                              "[ \"America/Regina\", \"Regina\", \"50.40\", \"-104.65\" ],"
                              "[ \"America/Resolute\", \"Resolute\", \"74.70\", \"-94.83\" ],"
                              "[ \"America/Rio_Branco\", \"Rio Branco\", \"-9.97\", \"-67.80\" ],"
                              "[ \"America/Santarem\", \"Santarem\", \"-2.43\", \"-54.87\" ],"
                              "[ \"America/Santiago\", \"Santiago\", \"-33.45\", \"-70.67\" ],"
                              "[ \"America/Santo_Domingo\", \"Santo Domingo\", \"18.47\", \"-69.90\" ],"
                              "[ \"America/Sao_Paulo\", \"Sao Paulo\", \"-23.53\", \"-46.62\" ],"
                              "[ \"America/Scoresbysund\", \"Ittoqqortoormiit\", \"70.48\", \"-21.97\" ],"
                              "[ \"America/Sitka\", \"Sitka\", \"57.18\", \"-135.30\" ],"
                              "[ \"America/St_Barthelemy\", \"Saint-Barthélemy\", \"17.88\", \"-62.85\" ],"
                              "[ \"America/St_Johns\", \"St. John's\", \"47.57\", \"-52.72\" ],"
                              "[ \"America/St_Kitts\", \"Basseterre\", \"17.30\", \"-62.72\" ],"
                              "[ \"America/St_Lucia\", \"Castries\", \"14.02\", \"-61.00\" ],"
                              "[ \"America/St_Thomas\", \"Charlotte Amalie\", \"18.35\", \"-64.93\" ],"
                              "[ \"America/St_Vincent\", \"Kingstown\", \"13.15\", \"-61.23\" ],"
                              "[ \"America/Swift_Current\", \"Swift Current\", \"50.28\", \"-107.83\" ],"
                              "[ \"America/Tegucigalpa\", \"Tegucigalpa\", \"14.10\", \"-87.22\" ],"
                              "[ \"America/Thule\", \"Pituffik\", \"76.57\", \"-68.78\" ],"
                              "[ \"America/Thunder_Bay\", \"Thunder Bay\", \"43.65\", \"-79.38\" ],"
                              "[ \"America/Tijuana\", \"Tijuana\", \"32.53\", \"-117.02\" ],"
                              "[ \"America/Toronto\", \"Toronto\", \"43.65\", \"-79.38\" ],"
                              "[ \"America/Tortola\", \"Tortola\", \"18.45\", \"-64.62\" ],"
                              "[ \"America/Vancouver\", \"Vancouver\", \"49.27\", \"-123.12\" ],"
                              "[ \"America/Whitehorse\", \"Whitehorse\", \"60.72\", \"-135.05\" ],"
                              "[ \"America/Winnipeg\", \"Winnipeg\", \"49.88\", \"-97.15\" ],"
                              "[ \"America/Yakutat\", \"Yakutat\", \"59.55\", \"-139.73\" ],"
                              "[ \"America/Yellowknife\", \"Yellowknife\", \"53.55\", \"-113.47\" ],"
                              "[ \"Antarctica/Casey\", \"Casey\", \"-66.28\", \"110.52\" ],"
                              "[ \"Antarctica/Davis\", \"Davis\", \"-68.58\", \"77.97\" ],"
                              "[ \"Antarctica/DumontDUrville\", \"Dumont-d'Urville\", \"-66.67\", \"140.02\" ],"
                              "[ \"Antarctica/Macquarie\", \"Macquarie\", \"-54.50\", \"158.95\" ],"
                              "[ \"Antarctica/Mawson\", \"Mawson\", \"-67.60\", \"62.88\" ],"
                              "[ \"Antarctica/McMurdo\", \"McMurdo\", \"-77.83\", \"166.60\" ],"
                              "[ \"Antarctica/Palmer\", \"Palmer\", \"-64.80\", \"-64.10\" ],"
                              "[ \"Antarctica/Rothera\", \"Rothera\", \"-67.57\", \"-68.13\" ],"
                              "[ \"Antarctica/Syowa\", \"Syowa\", \"-69.01\", \"39.59\" ],"
                              "[ \"Antarctica/Troll\", \"Troll\", \"-72.01\", \"2.53\" ],"
                              "[ \"Antarctica/Vostok\", \"Vostok\", \"-78.40\", \"106.90\" ],"
                              "[ \"Arctic/Longyearbyen\", \"Longyearbyen\", \"78.00\", \"16.00\" ],"
                              "[ \"Asia/Aden\", \"Aden\", \"12.75\", \"45.20\" ],"
                              "[ \"Asia/Almaty\", \"Almaty\", \"43.25\", \"76.95\" ],"
                              "[ \"Asia/Amman\", \"Amman\", \"31.95\", \"35.93\" ],"
                              "[ \"Asia/Anadyr\", \"Anadyr\", \"64.75\", \"177.48\" ],"
                              "[ \"Asia/Aqtau\", \"Aqtau\", \"44.52\", \"50.27\" ],"
                              "[ \"Asia/Aqtobe\", \"Aqtobe\", \"50.28\", \"57.17\" ],"
                              "[ \"Asia/Ashgabat\", \"Ashgabat\", \"37.95\", \"58.38\" ],"
                              "[ \"Asia/Atyrau\", \"Atyrau\", \"47.12\", \"51.93\" ],"
                              "[ \"Asia/Baghdad\", \"Baghdad\", \"33.35\", \"44.42\" ],"
                              "[ \"Asia/Bahrain\", \"Bahrain\", \"26.38\", \"50.58\" ],"
                              "[ \"Asia/Baku\", \"Baku\", \"40.38\", \"49.85\" ],"
                              "[ \"Asia/Bangkok\", \"Bangkok\", \"13.75\", \"100.52\" ],"
                              "[ \"Asia/Barnaul\", \"Barnaul\", \"53.37\", \"83.75\" ],"
                              "[ \"Asia/Beirut\", \"Beirut\", \"33.88\", \"35.50\" ],"
                              "[ \"Asia/Bishkek\", \"Bishkek\", \"42.90\", \"74.60\" ],"
                              "[ \"Asia/Brunei\", \"Brunei\", \"4.93\", \"114.92\" ],"
                              "[ \"Asia/Chita\", \"Chita\", \"52.05\", \"113.47\" ],"
                              "[ \"Asia/Choibalsan\", \"Choibalsan\", \"47.92\", \"106.88\" ],"
                              "[ \"Asia/Colombo\", \"Colombo\", \"6.93\", \"79.85\" ],"
                              "[ \"Asia/Damascus\", \"Damascus\", \"33.50\", \"36.30\" ],"
                              "[ \"Asia/Dhaka\", \"Dhaka\", \"23.72\", \"90.42\" ],"
                              "[ \"Asia/Dili\", \"Dili\", \"-8.55\", \"125.58\" ],"
                              "[ \"Asia/Dubai\", \"Dubai\", \"25.30\", \"55.30\" ],"
                              "[ \"Asia/Dushanbe\", \"Dushanbe\", \"38.58\", \"68.80\" ],"
                              "[ \"Asia/Famagusta\", \"Famagusta\", \"35.12\", \"33.95\" ],"
                              "[ \"Asia/Gaza\", \"Gaza\", \"31.50\", \"34.47\" ],"
                              "[ \"Asia/Hebron\", \"Hebron\", \"31.53\", \"35.09\" ],"
                              "[ \"Asia/Ho_Chi_Minh\", \"Ho Chi Minh City\", \"10.75\", \"106.67\" ],"
                              "[ \"Asia/Hong_Kong\", \"Hong Kong\", \"22.28\", \"114.15\" ],"
                              "[ \"Asia/Hovd\", \"Hovd\", \"48.02\", \"91.65\" ],"
                              "[ \"Asia/Irkutsk\", \"Irkutsk\", \"52.27\", \"104.33\" ],"
                              "[ \"Asia/Jakarta\", \"Jakarta\", \"-6.17\", \"106.80\" ],"
                              "[ \"Asia/Jayapura\", \"Jayapura\", \"-2.53\", \"140.70\" ],"
                              "[ \"Asia/Jerusalem\", \"Jerusalem\", \"31.78\", \"35.22\" ],"
                              "[ \"Asia/Kabul\", \"Kabul\", \"34.52\", \"69.20\" ],"
                              "[ \"Asia/Kamchatka\", \"Kamchatka\", \"53.02\", \"158.65\" ],"
                              "[ \"Asia/Karachi\", \"Karachi\", \"24.87\", \"67.05\" ],"
                              "[ \"Asia/Kathmandu\", \"Kathmandu\", \"27.72\", \"85.32\" ],"
                              "[ \"Asia/Khandyga\", \"Khandyga\", \"62.66\", \"135.55\" ],"
                              "[ \"Asia/Kolkata\", \"Kolkata\", \"22.57\", \"88.36\" ],"
                              "[ \"Asia/Krasnoyarsk\", \"Krasnoyarsk\", \"56.02\", \"92.83\" ],"
                              "[ \"Asia/Kuala_Lumpur\", \"Kuala Lumpur\", \"3.17\", \"101.70\" ],"
                              "[ \"Asia/Kuching\", \"Kuching\", \"1.55\", \"110.33\" ],"
                              "[ \"Asia/Kuwait\", \"Kuwait\", \"29.33\", \"47.98\" ],"
                              "[ \"Asia/Macau\", \"Macau\", \"22.20\", \"113.54\" ],"
                              "[ \"Asia/Magadan\", \"Magadan\", \"59.57\", \"150.80\" ],"
                              "[ \"Asia/Makassar\", \"Makassar\", \"-5.12\", \"119.40\" ],"
                              "[ \"Asia/Manila\", \"Manila\", \"14.59\", \"120.97\" ],"
                              "[ \"Asia/Muscat\", \"Muscat\", \"23.60\", \"58.58\" ],"
                              "[ \"Asia/Nicosia\", \"Nicosia\", \"35.17\", \"33.37\" ],"
                              "[ \"Asia/Novokuznetsk\", \"Novokuznetsk\", \"53.75\", \"87.12\" ],"
                              "[ \"Asia/Novosibirsk\", \"Novosibirsk\", \"55.03\", \"82.92\" ],"
                              "[ \"Asia/Omsk\", \"Omsk\", \"55.00\", \"73.40\" ],"
                              "[ \"Asia/Oral\", \"Oral\", \"51.22\", \"51.35\" ],"
                              "[ \"Asia/Phnom_Penh\", \"Phnom Penh\", \"11.55\", \"104.92\" ],"
                              "[ \"Asia/Pontianak\", \"Pontianak\", \"-0.03\", \"109.33\" ],"
                              "[ \"Asia/Pyongyang\", \"Pyongyang\", \"39.02\", \"125.75\" ],"
                              "[ \"Asia/Qatar\", \"Qatar\", \"25.28\", \"51.53\" ],"
                              "[ \"Asia/Qyzylorda\", \"Qyzylorda\", \"44.80\", \"65.47\" ],"
                              "[ \"Asia/Riyadh\", \"Riyadh\", \"24.63\", \"46.72\" ],"
                              "[ \"Asia/Sakhalin\", \"Sakhalin\", \"46.97\", \"142.70\" ],"
                              "[ \"Asia/Samarkand\", \"Samarkand\", \"39.67\", \"66.80\" ],"
                              "[ \"Asia/Seoul\", \"Seoul\", \"37.57\", \"126.98\" ],"
                              "[ \"Asia/Shanghai\", \"Shanghai\", \"31.23\", \"121.47\" ],"
                              "[ \"Asia/Singapore\", \"Singapore\", \"1.28\", \"103.85\" ],"
                              "[ \"Asia/Srednekolymsk\", \"Srednekolymsk\", \"67.47\", \"153.72\" ],"
                              "[ \"Asia/Taipei\", \"Taipei\", \"25.05\", \"121.50\" ],"
                              "[ \"Asia/Tashkent\", \"Tashkent\", \"41.33\", \"69.30\" ],"
                              "[ \"Asia/Tbilisi\", \"Tbilisi\", \"41.72\", \"44.82\" ],"
                              "[ \"Asia/Tehran\", \"Tehran\", \"35.67\", \"51.43\" ],"
                              "[ \"Asia/Thimphu\", \"Thimphu\", \"27.47\", \"89.65\" ],"
                              "[ \"Asia/Tokyo\", \"Tokyo\", \"35.68\", \"139.69\" ],"
                              "[ \"Asia/Tomsk\", \"Tomsk\", \"56.50\", \"84.97\" ],"
                              "[ \"Asia/Ulaanbaatar\", \"Ulaanbaatar\", \"47.92\", \"106.88\" ],"
                              "[ \"Asia/Urumqi\", \"Urumqi\", \"43.80\", \"87.58\" ],"
                              "[ \"Asia/Ust-Nera\", \"Ust-Nera\", \"64.56\", \"143.23\" ],"
                              "[ \"Asia/Vientiane\", \"Vientiane\", \"17.97\", \"102.60\" ],"
                              "[ \"Asia/Vladivostok\", \"Vladivostok\", \"43.17\", \"131.93\" ],"
                              "[ \"Asia/Yakutsk\", \"Yakutsk\", \"62.00\", \"129.67\" ],"
                              "[ \"Asia/Yangon\", \"Yangon\", \"16.78\", \"96.17\" ],"
                              "[ \"Asia/Yekaterinburg\", \"Yekaterinburg\", \"56.85\", \"60.60\" ],"
                              "[ \"Asia/Yerevan\", \"Yerevan\", \"40.18\", \"44.50\" ],"
                              "[ \"Atlantic/Azores\", \"Azores\", \"37.73\", \"-25.67\" ],"
                              "[ \"Atlantic/Bermuda\", \"Bermuda\", \"32.28\", \"-64.77\" ],"
                              "[ \"Atlantic/Canary\", \"Canary Islands\", \"28.10\", \"-15.40\" ],"
                              "[ \"Atlantic/Cape_Verde\", \"Cape Verde\", \"14.92\", \"-23.52\" ],"
                              "[ \"Atlantic/Faroe\", \"Faroe Islands\", \"62.02\", \"-6.77\" ],"
                              "[ \"Atlantic/Madeira\", \"Madeira\", \"32.63\", \"-16.90\" ],"
                              "[ \"Atlantic/Reykjavik\", \"Reykjavik\", \"64.15\", \"-21.85\" ],"
                              "[ \"Atlantic/South_Georgia\", \"South Georgia\", \"-54.27\", \"-36.53\" ],"
                              "[ \"Atlantic/Stanley\", \"Stanley\", \"-51.70\", \"-57.85\" ],"
                              "[ \"Atlantic/St_Helena\", \"St Helena\", \"-15.92\", \"-5.70\" ],"
                              "[ \"Australia/Adelaide\", \"Adelaide\", \"-34.92\", \"138.58\" ],"
                              "[ \"Australia/Brisbane\", \"Brisbane\", \"-27.47\", \"153.03\" ],"
                              "[ \"Australia/Broken_Hill\", \"Broken Hill\", \"-31.95\", \"141.45\" ],"
                              "[ \"Australia/Currie\", \"Currie\", \"-42.88\", \"147.32\" ],"
                              "[ \"Australia/Darwin\", \"Darwin\", \"-12.47\", \"130.83\" ],"
                              "[ \"Australia/Eucla\", \"Eucla\", \"-31.72\", \"128.87\" ],"
                              "[ \"Australia/Hobart\", \"Hobart\", \"-42.88\", \"147.32\" ],"
                              "[ \"Australia/Lindeman\", \"Lindeman\", \"-20.27\", \"149.00\" ],"
                              "[ \"Australia/Lord_Howe\", \"Lord Howe Island\", \"-31.55\", \"159.08\" ],"
                              "[ \"Australia/Melbourne\", \"Melbourne\", \"-37.81\", \"144.96\" ],"
                              "[ \"Australia/Perth\", \"Perth\", \"-31.95\", \"115.86\" ],"
                              "[ \"Australia/Sydney\", \"Sydney\", \"-33.87\", \"151.21\" ],"
                              "[ \"Europe/Amsterdam\", \"Amsterdam\", \"52.37\", \"4.90\" ],"
                              "[ \"Europe/Andorra\", \"Andorra la Vella\", \"42.50\", \"1.52\" ],"
                              "[ \"Europe/Astrakhan\", \"Astrakhan\", \"46.35\", \"48.05\" ],"
                              "[ \"Europe/Athens\", \"Athens\", \"37.97\", \"23.72\" ],"
                              "[ \"Europe/Belgrade\", \"Belgrade\", \"44.83\", \"20.50\" ],"
                              "[ \"Europe/Berlin\", \"Berlin\", \"52.52\", \"13.41\" ],"
                              "[ \"Europe/Bratislava\", \"Bratislava\", \"48.15\", \"17.12\" ],"
                              "[ \"Europe/Brussels\", \"Brussels\", \"50.85\", \"4.35\" ],"
                              "[ \"Europe/Bucharest\", \"Bucharest\", \"44.43\", \"26.10\" ],"
                              "[ \"Europe/Budapest\", \"Budapest\", \"47.50\", \"19.08\" ],"
                              "[ \"Europe/Busingen\", \"Busingen\", \"47.70\", \"8.68\" ],"
                              "[ \"Europe/Chisinau\", \"Chisinau\", \"47.00\", \"28.83\" ],"
                              "[ \"Europe/Copenhagen\", \"Copenhagen\", \"55.67\", \"12.58\" ],"
                              "[ \"Europe/Dublin\", \"Dublin\", \"53.33\", \"-6.25\" ],"
                              "[ \"Europe/Gibraltar\", \"Gibraltar\", \"36.13\", \"-5.35\" ],"
                              "[ \"Europe/Guernsey\", \"Guernsey\", \"49.45\", \"-2.54\" ],"
                              "[ \"Europe/Helsinki\", \"Helsinki\", \"60.17\", \"24.97\" ],"
                              "[ \"Europe/Isle_of_Man\", \"Isle of Man\", \"54.15\", \"-4.47\" ],"
                              "[ \"Europe/Istanbul\", \"Istanbul\", \"41.02\", \"28.97\" ],"
                              "[ \"Europe/Jersey\", \"Jersey\", \"49.18\", \"-2.11\" ],"
                              "[ \"Europe/Kaliningrad\", \"Kaliningrad\", \"54.72\", \"20.50\" ],"
                              "[ \"Europe/Kiev\", \"Kyiv\", \"50.43\", \"30.52\" ],"
                              "[ \"Europe/Kirov\", \"Kirov\", \"58.60\", \"49.65\" ],"
                              "[ \"Europe/Lisbon\", \"Lisbon\", \"38.72\", \"-9.13\" ],"
                              "[ \"Europe/Ljubljana\", \"Ljubljana\", \"46.05\", \"14.52\" ],"
                              "[ \"Europe/London\", \"London\", \"51.51\", \"-0.13\" ],"
                              "[ \"Europe/Luxembourg\", \"Luxembourg\", \"49.60\", \"6.15\" ],"
                              "[ \"Europe/Madrid\", \"Madrid\", \"40.42\", \"-3.70\" ],"
                              "[ \"Europe/Malta\", \"Malta\", \"35.90\", \"14.52\" ],"
                              "[ \"Europe/Mariehamn\", \"Mariehamn\", \"60.10\", \"19.95\" ],"
                              "[ \"Europe/Minsk\", \"Minsk\", \"53.90\", \"27.57\" ],"
                              "[ \"Europe/Monaco\", \"Monaco\", \"43.70\", \"7.38\" ],"
                              "[ \"Europe/Moscow\", \"Moscow\", \"55.76\", \"37.62\" ],"
                              "[ \"Europe/Oslo\", \"Oslo\", \"59.92\", \"10.75\" ],"
                              "[ \"Europe/Paris\", \"Paris\", \"48.86\", \"2.35\" ],"
                              "[ \"Europe/Podgorica\", \"Podgorica\", \"42.43\", \"19.27\" ],"
                              "[ \"Europe/Prague\", \"Prague\", \"50.08\", \"14.44\" ],"
                              "[ \"Europe/Riga\", \"Riga\", \"56.95\", \"24.10\" ],"
                              "[ \"Europe/Rome\", \"Rome\", \"41.90\", \"12.50\" ],"
                              "[ \"Europe/Samara\", \"Samara\", \"53.20\", \"50.15\" ],"
                              "[ \"Europe/San_Marino\", \"San Marino\", \"43.92\", \"12.47\" ],"
                              "[ \"Europe/Sarajevo\", \"Sarajevo\", \"43.87\", \"18.42\" ],"
                              "[ \"Europe/Saratov\", \"Saratov\", \"51.57\", \"46.03\" ],"
                              "[ \"Europe/Simferopol\", \"Simferopol\", \"44.95\", \"34.10\" ],"
                              "[ \"Europe/Skopje\", \"Skopje\", \"41.98\", \"21.43\" ],"
                              "[ \"Europe/Sofia\", \"Sofia\", \"42.68\", \"23.32\" ],"
                              "[ \"Europe/Stockholm\", \"Stockholm\", \"59.33\", \"18.05\" ],"
                              "[ \"Europe/Tallinn\", \"Tallinn\", \"59.42\", \"24.75\" ],"
                              "[ \"Europe/Tirane\", \"Tirane\", \"41.33\", \"19.83\" ],"
                              "[ \"Europe/Ulyanovsk\", \"Ulyanovsk\", \"54.33\", \"48.40\" ],"
                              "[ \"Europe/Uzhgorod\", \"Uzhhorod\", \"50.43\", \"30.52\" ],"
                              "[ \"Europe/Vaduz\", \"Vaduz\", \"47.15\", \"9.52\" ],"
                              "[ \"Europe/Vatican\", \"Vatican City\", \"41.90\", \"12.45\" ],"
                              "[ \"Europe/Vienna\", \"Vienna\", \"48.21\", \"16.37\" ],"
                              "[ \"Europe/Vilnius\", \"Vilnius\", \"54.68\", \"25.32\" ],"
                              "[ \"Europe/Volgograd\", \"Volgograd\", \"48.73\", \"44.42\" ],"
                              "[ \"Europe/Warsaw\", \"Warsaw\", \"52.23\", \"21.01\" ],"
                              "[ \"Europe/Zagreb\", \"Zagreb\", \"45.80\", \"15.97\" ],"
                              "[ \"Europe/Zaporozhye\", \"Zaporizhzhia\", \"50.43\", \"30.52\" ],"
                              "[ \"Europe/Zurich\", \"Zurich\", \"47.38\", \"8.54\" ],"
                              "[ \"Indian/Antananarivo\", \"Antananarivo\", \"-18.92\", \"47.52\" ],"
                              "[ \"Indian/Chagos\", \"Diego Garcia\", \"-7.33\", \"72.42\" ],"
                              "[ \"Indian/Christmas\", \"Flying Fish Cove\", \"-10.42\", \"105.72\" ],"
                              "[ \"Indian/Cocos\", \"West Island\", \"-12.17\", \"96.92\" ],"
                              "[ \"Indian/Comoro\", \"Comoro\", \"-11.68\", \"43.27\" ],"
                              "[ \"Indian/Kerguelen\", \"Port-aux-Français\", \"-49.35\", \"70.22\" ],"
                              "[ \"Indian/Mahe\", \"Victoria\", \"-4.67\", \"55.47\" ],"
                              "[ \"Indian/Maldives\", \"Malé\", \"4.17\", \"73.50\" ],"
                              "[ \"Indian/Mauritius\", \"Mauritius\", \"-20.17\", \"57.50\" ],"
                              "[ \"Indian/Mayotte\", \"Mayotte\", \"-12.78\", \"45.23\" ],"
                              "[ \"Indian/Reunion\", \"Réunion\", \"-20.87\", \"55.47\" ],"
                              "[ \"Pacific/Apia\", \"Apia\", \"-13.83\", \"-171.73\" ],"
                              "[ \"Pacific/Auckland\", \"Auckland\", \"-36.87\", \"174.77\" ],"
                              "[ \"Pacific/Bougainville\", \"Bougainville\", \"-6.22\", \"155.57\" ],"
                              "[ \"Pacific/Chatham\", \"Chatham Islands\", \"-43.95\", \"-176.55\" ],"
                              "[ \"Pacific/Chuuk\", \"Chuuk\", \"7.42\", \"151.78\" ],"
                              "[ \"Pacific/Easter\", \"Easter\", \"-27.15\", \"-109.43\" ],"
                              "[ \"Pacific/Efate\", \"Efate\", \"-17.67\", \"168.42\" ],"
                              "[ \"Pacific/Enderbury\", \"Enderbury\", \"-2.78\", \"-171.72\" ],"
                              "[ \"Pacific/Fakaofo\", \"Fakaofo\", \"-9.37\", \"-171.23\" ],"
                              "[ \"Pacific/Fiji\", \"Fiji\", \"-18.13\", \"178.42\" ],"
                              "[ \"Pacific/Funafuti\", \"Funafuti\", \"-8.52\", \"179.22\" ],"
                              "[ \"Pacific/Galapagos\", \"Galapagos\", \"-0.90\", \"-89.60\" ],"
                              "[ \"Pacific/Gambier\", \"Gambier\", \"-23.13\", \"-134.95\" ],"
                              "[ \"Pacific/Guadalcanal\", \"Guadalcanal\", \"-9.53\", \"160.20\" ],"
                              "[ \"Pacific/Guam\", \"Guam\", \"13.47\", \"144.75\" ],"
                              "[ \"Pacific/Honolulu\", \"Honolulu\", \"21.31\", \"-157.86\" ],"
                              "[ \"Pacific/Kiritimati\", \"Kiritimati\", \"1.87\", \"-157.33\" ],"
                              "[ \"Pacific/Kosrae\", \"Kosrae\", \"5.32\", \"162.98\" ],"
                              "[ \"Pacific/Kwajalein\", \"Kwajalein\", \"9.08\", \"167.33\" ],"
                              "[ \"Pacific/Majuro\", \"Majuro\", \"7.15\", \"171.20\" ],"
                              "[ \"Pacific/Marquesas\", \"Marquesas\", \"-9.00\", \"-139.50\" ],"
                              "[ \"Pacific/Midway\", \"Midway\", \"28.22\", \"-177.37\" ],"
                              "[ \"Pacific/Nauru\", \"Nauru\", \"-0.52\", \"166.92\" ],"
                              "[ \"Pacific/Niue\", \"Niue\", \"-19.02\", \"-169.92\" ],"
                              "[ \"Pacific/Norfolk\", \"Norfolk\", \"-29.05\", \"167.97\" ],"
                              "[ \"Pacific/Noumea\", \"Nouméa\", \"-22.27\", \"166.45\" ],"
                              "[ \"Pacific/Pago_Pago\", \"Pago Pago\", \"-14.27\", \"-170.70\" ],"
                              "[ \"Pacific/Palau\", \"Palau\", \"7.33\", \"134.48\" ],"
                              "[ \"Pacific/Pitcairn\", \"Adamstown\", \"-25.07\", \"-130.08\" ],"
                              "[ \"Pacific/Pohnpei\", \"Pohnpei\", \"6.97\", \"158.22\" ],"
                              "[ \"Pacific/Port_Moresby\", \"Port Moresby\", \"-9.50\", \"147.17\" ],"
                              "[ \"Pacific/Rarotonga\", \"Rarotonga\", \"-21.23\", \"-159.77\" ],"
                              "[ \"Pacific/Saipan\", \"Saipan\", \"15.20\", \"145.75\" ],"
                              "[ \"Pacific/Tahiti\", \"Tahiti\", \"-17.53\", \"-149.57\" ],"
                              "[ \"Pacific/Tarawa\", \"Tarawa\", \"1.42\", \"173.00\" ],"
                              "[ \"Pacific/Tongatapu\", \"Nukuʻalofa\", \"-21.13\", \"-175.20\" ],"
                              "[ \"Pacific/Wake\", \"Wake\", \"19.28\", \"166.62\" ],"
                              "[ \"Pacific/Wallis\", \"Mata-Utu\", \"-13.30\", \"-176.17\" ]]";
