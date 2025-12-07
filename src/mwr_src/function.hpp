#include "../common.h"

#pragma once

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void make_hardcopy_on_sd() {
    const uint8_t bmp320x240[70] = {
        0x42, 0x4D, 0x46, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x58, 0x02, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    const uint8_t bmp480x320[70] = {
        0x42, 0x4D, 0x46, 0xB0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 0x40, 0x01,
        0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x04, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
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

    File hc = SD_MMC.open("/hardcopy.bmp", "w", true);
    if (TFT_CONTROLLER < 2) {
        hc.write(bmp320x240, sizeof(bmp320x240));
        uint16_t buff[320];
        for (int i = 240; i > 0; i--) {
            tft.readRect(0, i - 1, 320, 1, buff);
            hc.write((uint8_t*)buff, 320 * 2);
        }
        hc.close();
    } else if (TFT_CONTROLLER < 7) {
        hc.write(bmp480x320, sizeof(bmp480x320));
        uint16_t buff[480];
        for (int i = 320; i > 0; i--) {
            tft.readRect(0, i - 1, 480, 1, buff);
            hc.write((uint8_t*)buff, 480 * 2);
        }
        hc.close();
    } else {
        hc.write(bmp800x480, sizeof(bmp800x480));
        uint16_t buff[800];
        for (int i = 480; i > 0; i--) {
            tft.readRect(0, i - 1, 800, 1, buff);
            hc.write((uint8_t*)buff, 800 * 2);
        }
        hc.close();
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void GetRunTimeStats(char* pcWriteBuffer) {
    TaskStatus_t* pxTaskStatusArray;
    UBaseType_t   uxArraySize;
    uint8_t       ulStatsAsPercentage;
    uint64_t      ulTotalRunTime;
    char          leftSpace[] = "             |";

    // Take a snapshot of the number of tasks in case it changes while this function is executing.
    uxArraySize = uxTaskGetNumberOfTasks();

    // Allocate a TaskStatus_t structure for each task.  An array could be allocated statically at compile time.
    pxTaskStatusArray = (TaskStatus_t*)pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

    if (pxTaskStatusArray != NULL) {
        // Generate raw status information about each task.
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, (UBaseType_t)uxArraySize, &ulTotalRunTime);

        // For percentage calculations.
        ulTotalRunTime /= 100UL;

        char* tmpBuff = x_ps_malloc(100);
        strcpy(pcWriteBuffer, leftSpace);
        strcat(pcWriteBuffer, ANSI_ESC_YELLOW " TASKNAME            | RUNTIMECOUNTER | TOTALRUNTIME[%] | CORE | PRIO  |\n");
        strcat(pcWriteBuffer, leftSpace);
        strcat(pcWriteBuffer, "---------------------+----------------+-----------------+------+-------|\n");

        // Avoid divide by zero errors.
        if (ulTotalRunTime > 0) {
            // For each populated position in the pxTaskStatusArray array, format the raw data as human readable ASCII data
            for (int x = 0; x < uxArraySize; x++) {
                // What percentage of the total run time has the task used? This will always be rounded down to the nearest integer.
                // ulTotalRunTimeDiv100 has already been divided by 100.
                ulStatsAsPercentage = pxTaskStatusArray[x].ulRunTimeCounter / ulTotalRunTime;
                memset(tmpBuff, 0x20, 100);
                memcpy(tmpBuff, pxTaskStatusArray[x].pcTaskName, strlen(pxTaskStatusArray[x].pcTaskName));
                tmpBuff[20] = '|';
                int8_t  core = (pxTaskStatusArray[x].xCoreID);
                uint8_t prio = (pxTaskStatusArray[x].uxBasePriority);
                if (ulStatsAsPercentage) {
                    sprintf(tmpBuff + 23, "%12lu  |       %02lu%%       |%4i  |%5d  |", (long unsigned int)pxTaskStatusArray[x].ulRunTimeCounter, (long unsigned int)ulStatsAsPercentage, core, prio);
                } else {
                    sprintf(tmpBuff + 23, "%12lu  |       <1%%       |%4i  |%5d  |", (long unsigned int)pxTaskStatusArray[x].ulRunTimeCounter, core, prio);
                }
                uint8_t i = 23;
                while (tmpBuff[i] == '0') {
                    tmpBuff[i] = ' ';
                    i++;
                }
                if (tmpBuff[45] == '0') tmpBuff[45] = ' ';
                strcat(pcWriteBuffer, leftSpace);
                strcat(pcWriteBuffer, " ");
                strcat(pcWriteBuffer, tmpBuff);
                strcat(pcWriteBuffer, "\n");
            }
            x_ps_free(&tmpBuff);
        }
        // The array is no longer needed, free the memory it consumes.
        vPortFree(pxTaskStatusArray);

#if TFT_CONTROLLER == 7
        extern uint64_t s_totalRuntime;
        tmpBuff = x_ps_malloc(130);
        if (s_totalRuntime > 0) {
            sprintf(tmpBuff, "%s" ANSI_ESC_LIGHTGREEN " time since start: %llus, VSYNCS: %llu  ==> fps: %llu", leftSpace, s_totalRuntime, tft.getVsyncCounter(),
                    tft.getVsyncCounter() / s_totalRuntime);
        } else {
            sprintf(tmpBuff, "%s" ANSI_ESC_LIGHTGREEN " time since start: %llus, VSYNCS: %llu  ==> fps: <1", leftSpace, s_totalRuntime, tft.getVsyncCounter());
        }
        strcat(tmpBuff, "                                   ");
        tmpBuff[90] = '\0';
        strcat(tmpBuff, ANSI_ESC_YELLOW "|\n");
        strcat(pcWriteBuffer, tmpBuff);
        x_ps_free(&tmpBuff);
#endif
        strcat(pcWriteBuffer, "             |---------------------+----------------+-----------------+------+-------|\n");
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool get_esp_items(uint8_t* s_resetReason, bool* s_f_FFatFound) {

    ps_ptr<char> chipModel = ESP.getChipModel();
    uint8_t      avMajor = ESP_ARDUINO_VERSION_MAJOR;
    uint8_t      avMinor = ESP_ARDUINO_VERSION_MINOR;
    uint8_t      avPatch = ESP_ARDUINO_VERSION_PATCH;
    Serial.printf("ESP32 Chip: %s\n", chipModel.c_get());
    Serial.printf("Arduino Version: %d.%d.%d\n", avMajor, avMinor, avPatch);
    uint8_t idfMajor = ESP_IDF_VERSION_MAJOR;
    uint8_t idfMinor = ESP_IDF_VERSION_MINOR;
    uint8_t idfPatch = ESP_IDF_VERSION_PATCH;
    Serial.printf("ESP-IDF Version: %d.%d.%d\n", idfMajor, idfMinor, idfPatch);

    Serial.printf("ARDUINO_LOOP_STACK_SIZE %d words (32 bit)\n", CONFIG_ARDUINO_LOOP_STACK_SIZE);
    Serial.printf("FLASH size %lu bytes, speed %lu MHz\n", (long unsigned)ESP.getFlashChipSize(), (long unsigned)ESP.getFlashChipSpeed() / 1000000);
    Serial.printf("CPU speed %lu MHz\n", (long unsigned)ESP.getCpuFreqMHz());
    Serial.printf("SDMMC speed %d MHz\n", SDMMC_FREQUENCY / 1000000);
    Serial.printf("TFT speed %d MHz\n", TFT_FREQUENCY / 1000000);

    if (!psramInit()) {
        Serial.printf(ANSI_ESC_RED "PSRAM not found! MiniWebRadio doesn't work properly without PSRAM!" ANSI_ESC_WHITE);
    } else {
        Serial.printf("PSRAM total size: %lu bytes\n", (long unsigned)ESP.getPsramSize());
    }
    if (ESP.getFlashChipSize() > 80000000) {
        if (!FFat.begin()) {
            if (!FFat.format()) Serial.printf("FFat Mount Failed\n");
        } else {
            Serial.printf("FFat total space: %d bytes, free space: %d bytes", FFat.totalBytes(), FFat.freeBytes());
            *s_f_FFatFound = true;
        }
    }
    Serial.printf("Arduino is pinned to core %d\n", xPortGetCoreID());
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
    Serial.printf("RESET_REASON: %s\n", rr);
    Serial.print("\n");

    if (chipModel.equals("ESP32-S3")) {
    } // ...  okay
    else if (chipModel.equals("ESP32-P4")) {
    } // ...  okay
    else {
        SerialPrintfln(ANSI_ESC_RED "MiniWebRadio does not work with %s", chipModel.c_get());
        return false;
    }
    Serial.print("\n");
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
const char* aes_encrypt(const char* input) {
    static char* output = NULL;
    uint16_t     len = strlen(input) / 16;
    len++;
    x_ps_free(&output);
    output = (char*)x_ps_calloc((len * 16) + 1, 1);
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, (const unsigned char*)aesKey, 128);
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, (const unsigned char*)input, (unsigned char*)output);
    mbedtls_aes_free(&aes);
    return output;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char* aes_decrypt(const char* input) {
    static char* output = NULL;
    uint16_t     len = strlen(input) + 1;
    x_ps_free(&output);
    output = (char*)x_ps_calloc(len, 1);
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, (const unsigned char*)aesKey, 128);
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, (const unsigned char*)input, (unsigned char*)output);
    mbedtls_aes_free(&aes);
    return output;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌  I R _ B U T T O N S  📌📌📌
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
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
        const char* ptr = jsonString;
        uint8_t     buttonNr = 0;
        size_t      buttonIndex = 0;

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
                    // log_w("buttonNr %i, val %i, label %s", buttonNr, m_settings->irbuttons[buttonNr].val, m_settings->irbuttons[buttonNr].label);
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
        //    log_w("%s", jsonString.c_str());
        // JSON parsen
        if (!parseJSONString(jsonString.c_str())) {
            Serial.println("Failed to parse JSON.");
            return false;
        }
        // debug output
        m_numOfIrButtons = 0;
        while (true) {
            if (m_settings->irbuttons[m_numOfIrButtons].label == NULL) break;

            // if(m_settings->irbuttons[m_numOfIrButtons].val == -1) log_w("IR_buttonNr %02i, value -1,   label %s", m_numOfIrButtons, m_settings->irbuttons[m_numOfIrButtons].label);
            //  else log_w("IR_buttonNr %02i, value 0x%02X, label %s", m_numOfIrButtons, m_settings->irbuttons[m_numOfIrButtons].val, m_settings->irbuttons[m_numOfIrButtons].label);
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

    File         m_masterFile;
    File         m_slaveFile;
    ps_ptr<char> m_buff;
    ps_ptr<char> m_lastConnectedFile = "";
    ps_ptr<char> m_lastConnectedFolder = "";
    ps_ptr<char> m_lastConnectedFileName = "";
    ps_ptr<char> m_JSONstr;

  public:
    SD_content() { m_files.clear(); }
    ~SD_content() {
        m_files.clear();
        m_JSONstr.reset();
        m_lastConnectedFile.reset();
        m_lastConnectedFolder.reset();
        m_lastConnectedFileName.reset();
    }
    bool listFilesInDir(const char* path, boolean audioFilesOnly, boolean withoutDirs) {
        m_files.clear();
        if (m_masterFile) m_masterFile.close();
        if (!SD_MMC.exists(path)) {
            SerialPrintfln(ANSI_ESC_RED "SD_MMC/%s not exist", path);
            return false;
        }
        m_masterFile = SD_MMC.open(path);
        if (!m_masterFile.isDirectory()) {
            SerialPrintfln(ANSI_ESC_RED "SD_MMC/%s is not a directory", path);
            m_masterFile.close();
            return false;
        }
        while (true) { // get content
            m_slaveFile = m_masterFile.openNextFile();
            if (!m_slaveFile) break;
            if (m_slaveFile.isDirectory()) {
                if (!withoutDirs) { // folder size is -1
                    ps_ptr<char> path = m_slaveFile.path();
                    path.append("/"); // add '/'
                    m_files.emplace_back((int)-1, m_slaveFile.name(), path);
                }
            } else {
                if (audioFilesOnly) {
                    if (endsWith(m_slaveFile.name(), ".mp3") || endsWith(m_slaveFile.name(), ".aac") || endsWith(m_slaveFile.name(), ".m4a") || endsWith(m_slaveFile.name(), ".wav") ||
                        endsWith(m_slaveFile.name(), ".flac") || endsWith(m_slaveFile.name(), ".m3u") || endsWith(m_slaveFile.name(), ".opus") || endsWith(m_slaveFile.name(), ".ogg")) {
                        m_files.emplace_back(m_slaveFile.size(), m_slaveFile.name(), m_slaveFile.path());
                    }
                } else {
                    m_files.emplace_back(m_slaveFile.size(), m_slaveFile.name(), m_slaveFile.path());
                }
            }
        }
        sort();
        m_masterFile.close();
        return true;
    }

    bool isDir(uint16_t idx) {
        if (idx >= m_files.size()) {
            MWR_LOG_ERROR("idx %i is oor, max = %i", idx, m_files.size() - 1);
            return false;
        }
        if (m_files[idx].fileSize == -1) return true;
        return false;
    }
    size_t      getSize() { return m_files.size(); }
    const char* getColouredSStringByIndex(uint16_t idx) {
        if (m_files.size() == 0) {
            MWR_LOG_WARN("m_files.size() is 0");
            return "";
        }
        if (m_files.size() < idx + 1) {
            MWR_LOG_WARN("idx %i is oor, max = %i", idx, m_files.size());
            return "";
        }
        if (isDir(idx)) return m_files[idx].fileName.get();
        m_buff.assignf("%s" ANSI_ESC_YELLOW " %li", m_files[idx].fileName.c_get(), m_files[idx].fileSize);
        return m_buff.get();
    }
    const char* getFileNameByIndex(uint16_t idx) {
        if (m_files.size() == 0) {
            MWR_LOG_WARN("m_files.size() is 0");
            return "";
        }
        if (m_files.size() < idx + 1) {
            MWR_LOG_WARN("idx %i is oor, max = %i", idx, m_files.size());
            return "";
        }
        return m_files[idx].fileName.get();
    }

    int32_t getFileSizeByIndex(uint16_t idx) {
        if (m_files.size() == 0) {
            MWR_LOG_WARN("m_files.size() is 0");
            return 0;
        }
        if (m_files.size() < idx + 1) {
            MWR_LOG_WARN("idx %i is oor, max = %i", idx, m_files.size());
            return 0;
        }
        return m_files[idx].fileSize;
    }

    const char* getFilePathByIndex(uint16_t idx) {
        if (m_files.size() == 0) {
            MWR_LOG_WARN("m_files.size() is 0");
            return "";
        }
        if (m_files.size() < idx + 1) {
            MWR_LOG_WARN("idx %i is oor, max = %i", idx, m_files.size());
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
        return m_files[idx].filePath.c_get();
    }

    const char* getFileFolderByIndex(uint16_t idx) {
        if (m_files.size() == 0) {
            MWR_LOG_WARN("m_files.size() is 0");
            return "";
        }
        if (m_files.size() < idx + 1) {
            MWR_LOG_WARN("idx %i is oor, max = %i", idx, m_files.size());
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
        if (isDir(idx)) return m_files[idx].filePath.c_get();
        int lastSlashIndex = m_files[idx].filePath.last_index_of('/');
        m_buff.copy_from(m_files[idx].filePath.get());
        m_buff[lastSlashIndex + 1] = '\0';
        return m_buff.get();
    }

    int16_t getIndexByName(const char* path) {
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
        if (!path) return -1;
        for (int i = 0; i < m_files.size(); i++) {
            if (strcmp((const char*)m_files[i].filePath.get(), path) == 0) { return i; }
        }
        return -1;
    }

    uint16_t getNextAudioFile(uint16_t currIdx) { // assume listFilesInDir with "audioFilesOnly"
        if (m_files.size() == 0) return 0;
        if (currIdx >= m_files.size()) currIdx = m_files.size() - 1;
        int16_t newIdx = currIdx;
        while (true) {
            newIdx++;
            if (newIdx >= m_files.size()) newIdx = 0;
            if (newIdx == currIdx) break;                           // avoid an infinite loop
            if (!m_files[newIdx].fileName.ends_with(".m3u")) break; // skip m3u files
        }
        return newIdx;
    }

    uint16_t getPrevAudioFile(uint16_t currIdx) { // assume listFilesInDir with "audioFilesOnly"
        if (m_files.size() == 0) return 0;
        if (currIdx >= m_files.size()) currIdx = m_files.size() - 1;
        int16_t newIdx = currIdx;
        while (true) {
            newIdx--;
            if (newIdx == -1) newIdx = m_files.size() - 1;
            if (newIdx == currIdx) break;                           // avoid an infinite loop
            if (!m_files[newIdx].fileName.ends_with(".m3u")) break; // skip m3u files
        }
        return newIdx;
    }

    void setLastConnectedFile(const char* lastconnectedItem) {
        /*  lastconnectedItem                       m_lastConnectedFolder       m_lastConnectedFileName     m_lastConnectedFile
            "/audiofiles/wavfiles/chicken.wav"      "/audiofiles/wavfiles/"     "chicken.wav"               "/audiofiles/wavfiles/chicken.wav"
            "xyz/chicken.wav"                       "/audiofiles/"              ""                          "/audiofiles/"                      // does not start with "/"
            "/audiofiles/wavfiles/chickenwav"       "/audiofiles/wavfiles/"     ""                          "/audiofiles/wavfiles/"             // file has no extension
            "/chicken.wav"                          "/"                         "chicken.wav"               "/chicken.wav"                      // we have no folder
            "/audiofiles/wavfiles/"                 "/audiofiles/wavfiles/"     ""                          "/audiofiles/wavfiles/"             // we have no file
            "/audiofiles/wavfiles/.wav"             "/audiofiles/wavfiles/"     ""                          "/audiofiles/wavfiles/"             // file has no name
        */
        m_lastConnectedFileName.assign("");
        m_lastConnectedFolder.assign("");
        int posFirst = 0, posLast = 0, posDot = 0;
        if (!lastconnectedItem) { // guard, lastconnectedItem == NULL
            m_lastConnectedFileName.assign("");
            m_lastConnectedFolder.assign("/audiofiles/");
            goto exit;
        }
        posFirst = indexOf(lastconnectedItem, "/", 0);
        posLast = lastIndexOf(lastconnectedItem, '/');
        if (posFirst != 0) { // guard, does not start with /
            m_lastConnectedFileName.assign("");
            m_lastConnectedFolder.assign("/audiofiles/");
            goto exit;
        }
        if (posLast == 0) {
            m_lastConnectedFolder.assign("/");
        } // we have no folder name
        else {
            m_lastConnectedFolder.copy_from(lastconnectedItem, posLast + 1);
        }

        if (posLast == strlen(lastconnectedItem) - 1) {
            m_lastConnectedFileName.assign("");
        } // we have no file name
        else {
            m_lastConnectedFileName.copy_from(lastconnectedItem + posLast + 1);
        }

        // log_e("posFirst %i, posLast %i, m_lastConnectedFileName %s, m_lastConnectedFolder %s", posFirst, posLast, m_lastConnectedFileName, m_lastConnectedFolder);
        posDot = m_lastConnectedFileName.index_of('.', 0);
        if (posDot == -1) { // no extension
            m_lastConnectedFileName.assign("");
        }

    exit:
        m_lastConnectedFile.clone_from(m_lastConnectedFolder);
        m_lastConnectedFile.append(m_lastConnectedFileName.c_get());
        MWR_LOG_DEBUG("lastconnectedItem %s", lastconnectedItem);
        MWR_LOG_DEBUG("lastConnectedFile %s", m_lastConnectedFile.c_get());
        MWR_LOG_DEBUG("m_lastConnectedFileName %s", m_lastConnectedFileName.c_get());
        MWR_LOG_DEBUG("m_lastConnectedFolder %s", m_lastConnectedFolder.c_get());
        listFilesInDir(m_lastConnectedFolder.c_get(), true, false);
        sort();
    }
    const char* getLastConnectedFolder() { return m_lastConnectedFolder.c_get(); }

    const char* getLastConnectedFileName() { return m_lastConnectedFileName.c_get(); }
    int         getPosByFileName(const char* fileName) {
        for (int i = 0; i < m_files.size(); i++) {
            if (m_files[i].fileName.equals(fileName)) return i; // fileName e.g. "file.mp3"
        }
        return -1;
    }

    const char* stringifyDirContent(ps_ptr<char> path) {
        ps_ptr<char> fileName;
        ps_ptr<char> fileSize;
        uint8_t      isDir = 0;
        if (!listFilesInDir(path.c_get(), false, false)) return "[]"; // if success: result will be in SD_content
        if (!getSize()) return "[]";                                  // empty?
        m_JSONstr.assign("[");

        for (int i = 0; i < getSize(); i++) { // build a JSON string in PSRAM, e.g. [{"name":"m","dir":true},{"name":"s","dir":true}]
            const char* fn = getColouredSStringByIndex(i);
            if (startsWith(fn, ".")) continue;    // ignore hidden folders
            int16_t idx = indexOf(fn, "\033", 1); // idx > 0 we have size (after ANSI ESC SEQUENCE)
            if (idx > 0) {
                isDir = 0; // {"name":"test.mp3","dir":false,"size":"3421"}
                fileName.copy_from(fn, idx);
                fileSize.copy_from(fn + idx + 6); // "all after 033[33m"
            } else {
                isDir = 1;
                fileName.copy_from(fn);
            }
            m_JSONstr.append("{\"name\":");
            m_JSONstr.appendf("\"%s\"", fileName.c_get());
            m_JSONstr.append(",\"dir\":");
            if (isDir) {
                m_JSONstr.append("true");
                m_JSONstr.append(",\"size\":0");
            } else {
                m_JSONstr.append("false");
                m_JSONstr.append(",\"size\":");
                m_JSONstr.appendf("%s", fileSize.c_get());
            }
            m_JSONstr.append("},");
            MWR_LOG_DEBUG("%s", fn);
        }
        int lastComma = m_JSONstr.last_index_of(',');
        m_JSONstr[lastComma] = ']'; // replace comma by square bracket close
        MWR_LOG_DEBUG("%s", m_JSONstr.c_get());
        return m_JSONstr.c_get();
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
    const char* getStationName(uint16_t staNr) {
        if (staNr > m_staCnt) return strdup("unknown");
        if (!m_stations.name[staNr]) return strdup("unknown");
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
            MWR_LOG_ERROR("wrong playlist path %s", path.c_get());
            return false;
        }
        if (!SD_MMC.exists(path.get())) {
            MWR_LOG_ERROR("Playlistfile %s not found", path.c_get());
            reset();
            return false;
        }
        m_playlist_file = SD_MMC.open(path.get());
        if (m_playlist_file.size() > 1048576) {
            MWR_LOG_ERROR("Playlist too big, size is %i", m_playlist_file.size());
            reset();
            return false;
        }
        int idx = path.last_index_of('/');
        if (idx != 0)
            m_playlist_path = path.substr(0, idx + 1);
        else
            m_playlist_path = path;

        ps_ptr<char> readBuff;
        ps_ptr<char> itemsBuff;
        ps_ptr<char> pathBuff;
        size_t       bytesRead = 0;
        bool         f_items_seen = false;
        bool         f_path_seen = false;
        readBuff.alloc(2024);
        itemsBuff.alloc(2024);
        pathBuff.alloc(2024);

        while (m_playlist_file.available() > 0) {

            bytesRead = m_playlist_file.readBytesUntil('\n', readBuff.get(), readBuff.size());
            if (bytesRead < 5) continue;

            readBuff[bytesRead] = '\0';
            trim(readBuff.get());

            if (readBuff.starts_with("#EXTM3U")) continue;
            if (readBuff.starts_with("#EXTINF:")) {
                itemsBuff = readBuff.substr(8);
                f_items_seen = true;
                f_path_seen = false;
            }

            if (readBuff.starts_with("#")) continue;
            f_path_seen = true;
            if (readBuff.starts_with("file://")) {
                pathBuff = readBuff.substr(7);
                pathBuff.urldecode();
            } else if (readBuff.starts_with_icase("http://") || readBuff.starts_with_icase("https://")) {
                pathBuff = readBuff;
            } else {
                if (!readBuff.starts_with("/"))
                    pathBuff = m_playlist_path + readBuff;
                else
                    pathBuff = readBuff;
                pathBuff.urldecode();
            }
            if (isAudio(pathBuff.c_get())) {
                if (f_path_seen) {
                    if (f_items_seen) {
                        int idx = itemsBuff.index_of(',');
                        if(idx != 0){
                            ps_ptr<char> t = "";
                            t = itemsBuff.substr(0, idx);
                            int d = t.to_uint32();
                            if(d > 0){ // has duration
                                MWR_LOG_WARN("duration %is", d);
                                itemsBuff = itemsBuff.substr(idx + 1);
                            }
                            itemsBuff.appendf(" " ANSI_ESC_YELLOW " %s" ANSI_ESC_RESET, t.c_get());
                        }

                        m_content_items.push_back(itemsBuff);
                    } else {
                        int idx = pathBuff.last_index_of('/');
                        if(idx != -1) itemsBuff = pathBuff.substr(idx + 1);
                        else itemsBuff = pathBuff;
                        m_content_items.push_back(itemsBuff);
                    }
                    m_content_file.push_back(pathBuff);
                }
                f_items_seen = false;
                f_path_seen = false;
                itemsBuff.clear();
                pathBuff.clear();
                readBuff.clear();
            }
        }
        m_playlist_file.close();

        // for (int i = 0; i < m_content_file.size(); i++) {
        //     MWR_LOG_WARN("%i, %s", i, m_content_file[i].c_get());
        //     MWR_LOG_INFO("%i, %s", i, m_content_items[i].c_get());
        // }
        return true;
    }

    bool create_playlist_from_SD_folder(ps_ptr<char> path) { // all files within a SD folder
        reset();
        if (!SD_MMC.exists(path.get())) {
            MWR_LOG_ERROR("SD_MMC/%s not exist", path);
            return false;
        }
        File folder = SD_MMC.open(path.get());
        if (!folder.isDirectory()) {
            MWR_LOG_ERROR("SD_MMC%s is not a directory", path);
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
                ps_ptr<char>name;
                name = file.name();
                name.appendf("" ANSI_ESC_YELLOW " %i" ANSI_ESC_RESET "", file.size());
                m_content_items.push_back(name);
            }
            file.close();
        }
        folder.close();

        // for (int i = 0; i < m_content_file.size(); i++) {
        //     MWR_LOG_WARN("%i, %s", i, m_content_file[i].c_get());  // path
        //     MWR_LOG_INFO("%i, %s", i, m_content_items[i].c_get()); // name
        // }
        return true;
    }

    bool create_playlist_from_DLNA_folder(const std::deque<DLNA_Client::srvItem>* foldercontent) {
        reset();
        for (int i = 0; i < foldercontent->size(); i++) {
            // log_i( "%d : (%d) %s %s -- %s",i, foldercontent.isAudio[i], foldercontent.itemURL[i], foldercontent.title[i], foldercontent.duration[i]);
            if (!foldercontent->at(i).isAudio) continue;
            uint16_t len =
                strlen((const char*)foldercontent->at(i).itemURL.c_get()) + strlen((const char*)foldercontent->at(i).title.c_get()) + strlen((const char*)foldercontent->at(i).duration.c_get()) + 3;
            ps_ptr<char> itstr(len);
            itstr = foldercontent->at(i).itemURL;
            itstr += "\n";
            itstr += foldercontent->at(i).duration;
            itstr += ",";
            itstr += foldercontent->at(i).title;
            MWR_LOG_DEBUG("pushing to playlist : %s", itstr.c_get());
            m_content_file.push_back(itstr);
        }
        if (!m_content_file.size()) return false;
        MWR_LOG_INFO("pls length %i", m_content_file.size());
        return true;
    }

    void sort_alphabetical() {
        for (int i = 0; i < m_content_file.size(); i++) { // easy bubble sort
            for (int j = 1; j < m_content_file.size(); j++) {
                if (strcmp(m_content_file[j - 1].c_get(), m_content_file[i].c_get()) > 0) {
                    swap(m_content_file[i], m_content_file[j - 1]);
                    swap(m_content_items[i], m_content_items[j - 1]);
                }
            }
        }
    }

    void sort_random() {
        for (int i = 0; i < m_content_file.size(); i++) { // easy bubble sort
            uint16_t randIndex = random(0, m_content_file.size());
            m_content_file[i].swap(m_content_file[randIndex]);   // swapping the values
            m_content_items[i].swap(m_content_items[randIndex]); // swapping the values
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
        if (m_index != -1) s.assignf(ANSI_ESC_CYAN "%s" ANSI_ESC_RESET, m_content_file[m_index]);
        s.println();
        return s;
    }

    ps_ptr<char> get_coloured_index() {
        ps_ptr<char> s = "";
        if (m_index != -1) s.assignf(ANSI_ESC_ORANGE "%03i/%03i" ANSI_ESC_RESET, m_index + 1, m_content_file.size());
        return s;
    }

    uint16_t get_size() { return m_content_file.size(); }
};
