// created: 10.02.2022
// updated: 19.05.2022

#include "common.h"
#include "SPIFFS.h"
#include "FFat.h"
#include "WiFiClientSecure.h"

#if DECODER == 0
    #include "vs1053_ext.h"     // see my repository at github "ESP32-vs1053_ext"
    VS1053 vs1053(VS1053_CS, VS1053_DCS, VS1053_DREQ, HSPI, VS1053_MOSI, VS1053_MISO, VS1053_SCK);
#endif

#if DECODER >= 1
    #include "Audio.h"     // see my repository at github "ESP32-audioI2S"
    Audio audio;
#endif

extern RTIME rtc;
extern SemaphoreHandle_t  mutex_rtc;

/***********************************************************************************************************************
*                                      A U D I O _ T A S K        V S 1 0 5 3                                          *
***********************************************************************************************************************/

#if DECODER == 0

enum : uint8_t { SET_VOLUME, GET_VOLUME, CONNECTTOHOST, CONNECTTOFS, STOPSONG, SETTONE, INBUFF_FILLED, INBUFF_FREE,
                 ISRUNNING};

struct audioMessage{
    uint8_t     cmd;
    const char* txt1;
    const char* txt2;
    const char* txt3;
    uint32_t    value1;
    uint32_t    value2;
    uint32_t    ret;
} audioTxMessage, audioRxMessage;

QueueHandle_t audioSetQueue = NULL;
QueueHandle_t audioGetQueue = NULL;

void CreateQueues(){
    audioSetQueue = xQueueCreate(10, sizeof(struct audioMessage));
    audioGetQueue = xQueueCreate(10, sizeof(struct audioMessage));
}

void audioTask(void *parameter) {
    CreateQueues();
    if(!audioSetQueue || !audioGetQueue){
        SerialPrintfln(ANSI_ESC_RED "Error: queues are not initialized");
        while(true){;}  // endless loop
    }

    struct audioMessage audioRxTaskMessage;
    struct audioMessage audioTxTaskMessage;

    vs1053.begin(); // Initialize VS1053 player
    uint32_t chipID = vs1053.printChipID();
    if(chipID == 0x00000000 || chipID == 0xFFFFFFFF){
        SerialPrintfln(ANSI_ESC_RED "Error: VS1053 not found");
        while(1){};
    }
    SerialPrintfln("VS1053 chipID = " ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE ", version = "
                                      ANSI_ESC_CYAN "%d", chipID, vs1053.printVersion());

    vs1053.setConnectionTimeout(1000, 4000);

    while(true){
        if(xQueueReceive(audioSetQueue, &audioRxTaskMessage, 1) == pdPASS) {
            if(audioRxTaskMessage.cmd == SET_VOLUME){
                audioTxTaskMessage.cmd = SET_VOLUME;
                vs1053.setVolume(audioRxTaskMessage.value1);
                audioTxTaskMessage.ret = 1;
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == CONNECTTOHOST){
                audioTxTaskMessage.cmd = CONNECTTOHOST;
                const char* host = audioRxTaskMessage.txt1;
                const char* user = audioRxTaskMessage.txt2;
                const char* pwd  = audioRxTaskMessage.txt3;
                audioTxTaskMessage.ret = vs1053.connecttohost(host, user, pwd);
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == CONNECTTOFS){
                audioTxTaskMessage.cmd = CONNECTTOFS;
                audioTxTaskMessage.ret = vs1053.connecttoFS(SD_MMC, audioRxTaskMessage.txt1, audioRxTaskMessage.value1);
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == GET_VOLUME){
                audioTxTaskMessage.cmd = GET_VOLUME;
                audioTxTaskMessage.ret = vs1053.getVolume();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == STOPSONG){
                audioTxTaskMessage.cmd = STOPSONG;
                audioTxTaskMessage.ret = vs1053.stop_mp3client();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == SETTONE){
                audioTxTaskMessage.cmd = SETTONE;
                uint8_t u8_tone[4];
                u8_tone[3] = (audioRxTaskMessage.value1 & 0xFF);       // toneha  Lower limit frequency in 10 Hz steps (2..15)
                u8_tone[2] = (audioRxTaskMessage.value1 >>  8) & 0xFF; // tonehf  Bass Enhancement in 1 dB steps (0..15, 0 = off)
                u8_tone[1] = (audioRxTaskMessage.value1 >> 16) & 0xFF; // tonela  Lower limit frequency in 1000 Hz steps (1..15)
                u8_tone[0] = (audioRxTaskMessage.value1 >> 24) & 0xFF; // tonelf  Treble Control in 1.5 dB steps (-8..7, 0 = off)
                //log_i("ha %d, hf %d, la %d, lf %d", u8_tone[0], u8_tone[1], u8_tone[2], u8_tone[3]);
                vs1053.setTone(u8_tone);
                audioTxTaskMessage.ret = 0;
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == INBUFF_FILLED){
                audioTxTaskMessage.cmd = INBUFF_FILLED;
                audioTxTaskMessage.ret = vs1053.bufferFilled();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == INBUFF_FREE){
                audioTxTaskMessage.cmd = INBUFF_FREE;
                audioTxTaskMessage.ret = vs1053.bufferFree();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == ISRUNNING){
                audioTxTaskMessage.cmd = ISRUNNING;
                audioTxTaskMessage.ret = vs1053.isRunning();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else{
                SerialPrintfln(ANSI_ESC_RED "Error: unknown audioTaskMessage");
            }
        }
        vs1053.loop();
    }
}

void audioInit() {
    xTaskCreatePinnedToCore(
        audioTask,             /* Function to implement the task */
        "audioplay",           /* Name of the task */
        8000,                  /* Stack size in words */
        NULL,                  /* Task input parameter */
        AUDIOTASK_PRIO,        /* Priority of the task */
        NULL,                  /* Task handle. */
        AUDIOTASK_CORE         /* Core where the task should run */
    );
    if(CORE_DEBUG_LEVEL >= 2){
        {SerialPrintfln("audiotask:   is pinned to core " ANSI_ESC_CYAN "%d", AUDIOTASK_CORE);}
        {SerialPrintfln("audiotask:   priority is " ANSI_ESC_CYAN "%d", AUDIOTASK_PRIO);}
    }
}

audioMessage transmitReceive(audioMessage msg){
    xQueueSend(audioSetQueue, &msg, portMAX_DELAY);
    if(xQueueReceive(audioGetQueue, &audioRxMessage, portMAX_DELAY) == pdPASS){
        if(msg.cmd != audioRxMessage.cmd){
            SerialPrintfln(ANSI_ESC_RED "Error: wrong reply from message queue");
        }
    }
    return audioRxMessage;
}

void audioSetVolume(uint8_t vol){
    audioTxMessage.cmd = SET_VOLUME;
    audioTxMessage.value1 = vol;
    audioMessage RX = transmitReceive(audioTxMessage);
    (void)RX;
}

uint8_t audioGetVolume(){
    audioTxMessage.cmd = GET_VOLUME;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

boolean audioConnecttohost(const char* host, const char* user, const char* pwd){
    audioTxMessage.cmd = CONNECTTOHOST;
    audioTxMessage.txt1 = host;
    audioTxMessage.txt2 = user;
    audioTxMessage.txt3 = pwd;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

boolean audioConnecttoFS(const char* filename, uint32_t resumeFilePos){
    audioTxMessage.cmd = CONNECTTOFS;
    audioTxMessage.txt1 = filename;
    audioTxMessage.value1 = resumeFilePos;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

uint32_t audioStopSong(){
    audioTxMessage.cmd = STOPSONG;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

void audioSetTone(int8_t toneha, int8_t tonehf, int8_t tonela, int8_t tonelf){
    audioTxMessage.cmd = SETTONE;
    audioTxMessage.value1 =  (uint8_t)toneha;
    audioTxMessage.value1 <<= 8;
    audioTxMessage.value1 += (uint8_t)tonehf;
    audioTxMessage.value1 <<= 8;
    audioTxMessage.value1 += (uint8_t)tonela;
    audioTxMessage.value1 <<= 8;
    audioTxMessage.value1 += (uint8_t)tonelf;
    audioMessage RX = transmitReceive(audioTxMessage);
    (void)RX;
}

uint32_t audioInbuffFilled(){
    audioTxMessage.cmd = INBUFF_FILLED;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

uint32_t audioInbuffFree(){
    audioTxMessage.cmd = INBUFF_FREE;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

boolean audioIsRunning(){
    audioTxMessage.cmd = ISRUNNING;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

#endif // DECODER == 0

/***********************************************************************************************************************
*                                          A U D I O _ T A S K     I 2 S                                               *
***********************************************************************************************************************/

#if DECODER >= 1

enum : uint8_t { SET_VOLUME, GET_VOLUME, CONNECTTOHOST, CONNECTTOFS, STOPSONG, SETTONE, INBUFF_FILLED, INBUFF_FREE,
                 ISRUNNING};

struct audioMessage{
    uint8_t     cmd;
    const char* txt1;
    const char* txt2;
    const char* txt3;
    uint32_t    value1;
    uint32_t    value2;
    uint32_t    ret;
} audioTxMessage, audioRxMessage;

QueueHandle_t audioSetQueue = NULL;
QueueHandle_t audioGetQueue = NULL;

void CreateQueues(){
    audioSetQueue = xQueueCreate(10, sizeof(struct audioMessage));
    audioGetQueue = xQueueCreate(10, sizeof(struct audioMessage));
}

void audioTask(void *parameter) {
    CreateQueues();
    if(!audioSetQueue || !audioGetQueue){
        SerialPrintfln(ANSI_ESC_RED "Error: queues are not initialized");
        while(true){;}  // endless loop
    }

    struct audioMessage audioRxTaskMessage;
    struct audioMessage audioTxTaskMessage;

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    if(I2S_MCLK != -1) audio.i2s_mclk_pin_select(I2S_MCLK);
    audio.setVolume(5); // 0...21
    audio.setConnectionTimeout(1000, 4000);

    while(true){
        if(xQueueReceive(audioSetQueue, &audioRxTaskMessage, 1) == pdPASS) {
            if(audioRxTaskMessage.cmd == SET_VOLUME){
                audioTxTaskMessage.cmd = SET_VOLUME;
                audio.setVolume(audioRxTaskMessage.value1);
                audioTxTaskMessage.ret = 1;
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == CONNECTTOHOST){
                audioTxTaskMessage.cmd = CONNECTTOHOST;
                const char* host = audioRxTaskMessage.txt1;
                const char* user = audioRxTaskMessage.txt2;
                const char* pwd  = audioRxTaskMessage.txt3;
                audioTxTaskMessage.ret = audio.connecttohost(host, user, pwd);
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == CONNECTTOFS){
                audioTxTaskMessage.cmd = CONNECTTOFS;
                audioTxTaskMessage.ret = audio.connecttoFS(SD_MMC, audioRxTaskMessage.txt1, audioRxTaskMessage.value1);
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == GET_VOLUME){
                audioTxTaskMessage.cmd = GET_VOLUME;
                audioTxTaskMessage.ret = audio.getVolume();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == STOPSONG){
                audioTxTaskMessage.cmd = STOPSONG;
                audioTxTaskMessage.ret = audio.stopSong();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == SETTONE){
                audioTxTaskMessage.cmd = SETTONE;
                int8_t lowPass, bandPass, highPass;
                lowPass  = (audioRxTaskMessage.value1 & 0xFF);
                bandPass = (audioRxTaskMessage.value1 >>  8) & 0xFF;
                highPass = (audioRxTaskMessage.value1 >> 16) & 0xFF;
                audio.setTone(lowPass, bandPass, highPass);
                audioTxTaskMessage.ret = 0;
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == INBUFF_FILLED){
                audioTxTaskMessage.cmd = INBUFF_FILLED;
                audioTxTaskMessage.ret = audio.inBufferFilled();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == INBUFF_FREE){
                audioTxTaskMessage.cmd = INBUFF_FREE;
                audioTxTaskMessage.ret = audio.inBufferFree();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == ISRUNNING){
                audioTxTaskMessage.cmd = ISRUNNING;
                audioTxTaskMessage.ret = audio.isRunning();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else{
                SerialPrintfln(ANSI_ESC_RED "Error: unknown audioTaskMessage");
            }
        }
        audio.loop();
    }
}

void audioInit() {
    xTaskCreatePinnedToCore(
        audioTask,              /* Function to implement the task */
        "audioplay",            /* Name of the task */
        7000,                   /* Stack size in words */
        NULL,                   /* Task input parameter */
        AUDIOTASK_PRIO,         /* Priority of the task */
        NULL,                   /* Task handle. */
        AUDIOTASK_CORE          /* Core where the task should run */
    );
    if(CORE_DEBUG_LEVEL >= 2){
        {SerialPrintfln("audiotask:   is pinned to core " ANSI_ESC_CYAN "%d", AUDIOTASK_CORE);}
        {SerialPrintfln("audiotask:   priority is " ANSI_ESC_CYAN "%d", AUDIOTASK_PRIO);}
    }
}

audioMessage transmitReceive(audioMessage msg){
    xQueueSend(audioSetQueue, &msg, portMAX_DELAY);
    if(xQueueReceive(audioGetQueue, &audioRxMessage, portMAX_DELAY) == pdPASS){
        if(msg.cmd != audioRxMessage.cmd){
            SerialPrintfln(ANSI_ESC_RED "Error: wrong reply from message queue");
        }
    }
    return audioRxMessage;
}

void audioSetVolume(uint8_t vol){
    audioTxMessage.cmd = SET_VOLUME;
    audioTxMessage.value1 = vol;
    audioMessage RX = transmitReceive(audioTxMessage);
    (void)RX;
}

uint8_t audioGetVolume(){
    audioTxMessage.cmd = GET_VOLUME;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

boolean audioConnecttohost(const char* host, const char* user, const char* pwd){
    audioTxMessage.cmd = CONNECTTOHOST;
    audioTxMessage.txt1 = host;
    audioTxMessage.txt2 = user;
    audioTxMessage.txt3 = pwd;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

boolean audioConnecttoFS(const char* filename, uint32_t resumeFilePos){
    audioTxMessage.cmd = CONNECTTOFS;
    audioTxMessage.txt1 = filename;
    audioTxMessage.value1 = resumeFilePos;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

uint32_t audioStopSong(){
    audioTxMessage.cmd = STOPSONG;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

void audioSetTone(int8_t lowPass, int8_t bandPass, int8_t highPass, int8_t unused){
    audioTxMessage.cmd = SETTONE;
    audioTxMessage.value1 = (uint8_t)highPass;
    audioTxMessage.value1 <<= 8;
    audioTxMessage.value1 += (uint8_t)bandPass;
    audioTxMessage.value1 <<= 8;
    audioTxMessage.value1 += (uint8_t)lowPass;
    audioMessage RX = transmitReceive(audioTxMessage);
    (void)unused;
    (void)RX;
}

uint32_t audioInbuffFilled(){
    audioTxMessage.cmd = INBUFF_FILLED;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

uint32_t audioInbuffFree(){
    audioTxMessage.cmd = INBUFF_FREE;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

boolean audioIsRunning(){
    audioTxMessage.cmd = ISRUNNING;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}
//
#endif // DECODER >= 1