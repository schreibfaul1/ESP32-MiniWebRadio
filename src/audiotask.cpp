// created: 10.02.2022
// updated: 13.02.2022

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


/***********************************************************************************************************************
*                                      A U D I O _ T A S K        V S 1 0 5 3                                          *
***********************************************************************************************************************/

#if DECODER == 0

enum : uint8_t {SET_VOLUME, GET_VOLUME, CONNECTTOHOST, CONNECTTOFS, STOPSONG, SETTONE, INBUFF_FILLED, INBUFF_FREE};

struct audioMessage{
    uint8_t     cmd;
    const char* txt;
    uint32_t    value;
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
        log_e("queues are not initialized");
        while(true){;}  // endless loop
    }

    struct audioMessage audioRxTaskMessage;
    struct audioMessage audioTxTaskMessage;

    vs1053.begin(); // Initialize VS1053 player
    const char* vs1053vers = vs1053.printVersion();
    if(!vs1053vers){
        log_e("VS1053 not found");
        while(1){};
    }
    Serial.printf("VS1053 %s\n", vs1053vers);

    while(true){
        if(xQueueReceive(audioSetQueue, &audioRxTaskMessage, 1) == pdPASS) {
            if(audioRxTaskMessage.cmd == SET_VOLUME){
                audioTxTaskMessage.cmd = SET_VOLUME;
                vs1053.setVolume(audioRxTaskMessage.value);
                audioTxTaskMessage.ret = 1;
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == CONNECTTOHOST){
                audioTxTaskMessage.cmd = CONNECTTOHOST;
                audioTxTaskMessage.ret = vs1053.connecttohost(audioRxTaskMessage.txt);
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == CONNECTTOFS){
                audioTxTaskMessage.cmd = CONNECTTOFS;
                audioTxTaskMessage.ret = vs1053.connecttoFS(SD_MMC, audioRxTaskMessage.txt, audioRxTaskMessage.value);
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
                u8_tone[3] = (audioRxTaskMessage.value & 0xFF);       // toneha  Lower limit frequency in 10 Hz steps (2..15)
                u8_tone[2] = (audioRxTaskMessage.value >>  8) & 0xFF; // tonehf  Bass Enhancement in 1 dB steps (0..15, 0 = off)
                u8_tone[1] = (audioRxTaskMessage.value >> 16) & 0xFF; // tonela  Lower limit frequency in 1000 Hz steps (1..15)
                u8_tone[0] = (audioRxTaskMessage.value >> 24) & 0xFF; // tonelf  Treble Control in 1.5 dB steps (-8..7, 0 = off)
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
            else{
                log_i("error");
            }
        }
        vs1053.loop();
    }
}

void audioInit() {
    xTaskCreatePinnedToCore(
        audioTask,           /* Function to implement the task */
        "audioplay",          /* Name of the task */
        5000,                  /* Stack size in words */
        NULL,                  /* Task input parameter */
        2 | portPRIVILEGE_BIT, /* Priority of the task */
        NULL,                  /* Task handle. */
        1                      /* Core where the task should run */
    );
}

audioMessage transmitReceive(audioMessage msg){
    xQueueSend(audioSetQueue, &msg, portMAX_DELAY);
    if(xQueueReceive(audioGetQueue, &audioRxMessage, portMAX_DELAY) == pdPASS){
        if(msg.cmd != audioRxMessage.cmd){
            log_e("wrong reply from message queue");
        }
    }
    return audioRxMessage;
}

void audioSetVolume(uint8_t vol){
    audioTxMessage.cmd = SET_VOLUME;
    audioTxMessage.value = vol;
    audioMessage RX = transmitReceive(audioTxMessage);
    (void)RX;
}

uint8_t audioGetVolume(){
    audioTxMessage.cmd = GET_VOLUME;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

boolean audioConnecttohost(const char* host){
    audioTxMessage.cmd = CONNECTTOHOST;
    audioTxMessage.txt = host;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

boolean audioConnecttoFS(const char* filename, uint32_t resumeFilePos){
    audioTxMessage.cmd = CONNECTTOFS;
    audioTxMessage.txt = filename;
    audioTxMessage.value = resumeFilePos;
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
    audioTxMessage.value =  (uint8_t)toneha;
    audioTxMessage.value <<= 8;
    audioTxMessage.value += (uint8_t)tonehf;
    audioTxMessage.value <<= 8;
    audioTxMessage.value += (uint8_t)tonela;
    audioTxMessage.value <<= 8;
    audioTxMessage.value += (uint8_t)tonelf;
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

#endif // DECODER == 0

/***********************************************************************************************************************
*                                          A U D I O _ T A S K     I 2 S                                               *
***********************************************************************************************************************/

#if DECODER >= 1

enum : uint8_t {SET_VOLUME, GET_VOLUME, CONNECTTOHOST, CONNECTTOFS, STOPSONG, SETTONE, INBUFF_FILLED, INBUFF_FREE};

struct audioMessage{
    uint8_t     cmd;
    const char* txt;
    uint32_t    value;
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
        log_e("queues are not initialized");
        while(true){;}  // endless loop
    }

    struct audioMessage audioRxTaskMessage;
    struct audioMessage audioTxTaskMessage;

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(5); // 0...21

    while(true){
        if(xQueueReceive(audioSetQueue, &audioRxTaskMessage, 1) == pdPASS) {
            if(audioRxTaskMessage.cmd == SET_VOLUME){
                audioTxTaskMessage.cmd = SET_VOLUME;
                audio.setVolume(audioRxTaskMessage.value);
                audioTxTaskMessage.ret = 1;
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == CONNECTTOHOST){
                audioTxTaskMessage.cmd = CONNECTTOHOST;
                audioTxTaskMessage.ret = audio.connecttohost(audioRxTaskMessage.txt);
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == CONNECTTOFS){
                audioTxTaskMessage.cmd = CONNECTTOFS;
                audioTxTaskMessage.ret = audio.connecttoFS(SD_MMC, audioRxTaskMessage.txt, audioRxTaskMessage.value);
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
                lowPass  = (audioRxTaskMessage.value & 0xFF);
                bandPass = (audioRxTaskMessage.value >>  8) & 0xFF;
                highPass = (audioRxTaskMessage.value >> 16) & 0xFF;
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
            else{
                log_i("error");
            }
        }
        audio.loop();
    }
}

void audioInit() {
    xTaskCreatePinnedToCore(
        audioTask,              /* Function to implement the task */
        "audioplay",            /* Name of the task */
        5000,                   /* Stack size in words */
        NULL,                   /* Task input parameter */
        2 | portPRIVILEGE_BIT,  /* Priority of the task */
        NULL,                   /* Task handle. */
        1                       /* Core where the task should run */
    );
}

audioMessage transmitReceive(audioMessage msg){
    xQueueSend(audioSetQueue, &msg, portMAX_DELAY);
    if(xQueueReceive(audioGetQueue, &audioRxMessage, portMAX_DELAY) == pdPASS){
        if(msg.cmd != audioRxMessage.cmd){
            log_e("wrong reply from message queue");
        }
    }
    return audioRxMessage;
}

void audioSetVolume(uint8_t vol){
    audioTxMessage.cmd = SET_VOLUME;
    audioTxMessage.value = vol;
    audioMessage RX = transmitReceive(audioTxMessage);
    (void)RX;
}

uint8_t audioGetVolume(){
    audioTxMessage.cmd = GET_VOLUME;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

boolean audioConnecttohost(const char* host){
    audioTxMessage.cmd = CONNECTTOHOST;
    audioTxMessage.txt = host;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

boolean audioConnecttoFS(const char* filename, uint32_t resumeFilePos){
    audioTxMessage.cmd = CONNECTTOFS;
    audioTxMessage.txt = filename;
    audioTxMessage.value = resumeFilePos;
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
    audioTxMessage.value = (uint8_t)highPass;
    audioTxMessage.value <<= 8;
    audioTxMessage.value += (uint8_t)bandPass;
    audioTxMessage.value <<= 8;
    audioTxMessage.value += (uint8_t)lowPass;
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
//
#endif // DECODER == 1