// created: 10.02.2022
// updated: 08.04.2024

#include "common.h"
#include "SPIFFS.h"
#include "FFat.h"
#include "WiFiClientSecure.h"
#include "Audio.h"     // see my repository at github "ESP32-audioI2S"

Audio audio;

extern RTIME rtc;
extern SemaphoreHandle_t  mutex_rtc;

enum : uint8_t { SET_VOLUME, GET_VOLUME, GET_BITRATE, CONNECTTOHOST, CONNECTTOFS, STOPSONG, SETTONE, INBUFF_FILLED,
                 INBUFF_FREE, INBUFF_SIZE, ISRUNNING, HIGHWATERMARK, GET_CODEC, PAUSERESUME, CONNECTION_TIMEOUT, GET_FILESIZE,
                 GET_FILEPOSITION, GET_VULEVEL, GET_AUDIOFILEDURATION, GET_AUDIOCURRENTTIME};

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

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT, I2S_MCLK);
    audio.setI2SCommFMT_LSB(I2S_COMM_FMT);
    audio.setVolume(5); // 0...21

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
            else if(audioRxTaskMessage.cmd == GET_BITRATE){
                audioTxTaskMessage.cmd = GET_BITRATE;
                audioTxTaskMessage.ret = audio.getBitRate(true);
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == GET_CODEC){
                audioTxTaskMessage.cmd = GET_CODEC;
                audioTxTaskMessage.ret = audio.getCodec();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == STOPSONG){
                audioTxTaskMessage.cmd = STOPSONG;
                audioTxTaskMessage.ret = audio.stopSong();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == SETTONE){
                audioTxTaskMessage.cmd = SETTONE;
                int8_t lowPass, bandPass, highPass, balance;
                lowPass  = (audioRxTaskMessage.value1 & 0xFF);
                bandPass = (audioRxTaskMessage.value1 >>  8) & 0xFF;
                highPass = (audioRxTaskMessage.value1 >> 16) & 0xFF;
                balance = (audioRxTaskMessage.value2 & 0xFF) * (-1);
                audio.setTone(lowPass, bandPass, highPass);
                audio.setBalance(balance);
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
            else if(audioRxTaskMessage.cmd == INBUFF_SIZE){
                audioTxTaskMessage.cmd = INBUFF_SIZE;
                audioTxTaskMessage.ret = 0;//audio.inBufferSize();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == ISRUNNING){
                audioTxTaskMessage.cmd = ISRUNNING;
                audioTxTaskMessage.ret = audio.isRunning();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == HIGHWATERMARK){
                audioTxTaskMessage.cmd = HIGHWATERMARK;
                audioTxTaskMessage.ret = uxTaskGetStackHighWaterMark(NULL);
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == PAUSERESUME){
                audioTxTaskMessage.cmd = PAUSERESUME;
                audioTxTaskMessage.ret = audio.pauseResume();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == CONNECTION_TIMEOUT){
                audioTxTaskMessage.cmd = CONNECTION_TIMEOUT;
                uint32_t to = audioRxTaskMessage.value1;
                uint32_t to_ssl = audioRxTaskMessage.value2;
                audio.setConnectionTimeout(to, to_ssl);
                audioTxTaskMessage.ret = 0;
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == GET_FILESIZE){
                audioTxTaskMessage.cmd = GET_FILESIZE;
                audioTxTaskMessage.ret = audio.getFileSize();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == GET_FILEPOSITION){
                audioTxTaskMessage.cmd = GET_FILEPOSITION;
                audioTxTaskMessage.ret = audio.getFilePos();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == GET_VULEVEL){
                audioTxTaskMessage.cmd = GET_VULEVEL;
                audioTxTaskMessage.ret = audio.getVUlevel();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == GET_AUDIOFILEDURATION){
                audioTxTaskMessage.cmd = GET_AUDIOFILEDURATION;
                audioTxTaskMessage.ret = audio.getAudioFileDuration();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == GET_AUDIOCURRENTTIME){
                audioTxTaskMessage.cmd = GET_AUDIOCURRENTTIME;
                audioTxTaskMessage.ret = audio.getAudioCurrentTime();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else{
                SerialPrintfln(ANSI_ESC_RED "Error: unknown audioTaskMessage");
            }
        }
        audio.loop();
    }
}

TaskHandle_t Task1;

void audioInit() {
    xTaskCreatePinnedToCore(
        audioTask,              /* Function to implement the task */
        "audioplay",            /* Name of the task */
        7500,                   /* Stack size in words */
        NULL,                   /* Task input parameter */
        AUDIOTASK_PRIO,         /* Priority of the task */
        &Task1,                 /* Task handle. */
        AUDIOTASK_CORE          /* Core where the task should run */
    );
    if(CORE_DEBUG_LEVEL >= 2){
        {SerialPrintfln("audiotask:   is pinned to core " ANSI_ESC_CYAN "%d", AUDIOTASK_CORE);}
        {SerialPrintfln("audiotask:   priority is " ANSI_ESC_CYAN "%d", AUDIOTASK_PRIO);}
    }
}

void audioTaskDelete(){
    vTaskDelete(Task1);
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

uint32_t audioGetBitRate(){
    audioTxMessage.cmd = GET_BITRATE;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

uint32_t audioGetCodec(){
    audioTxMessage.cmd = GET_CODEC;
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

void audioSetTone(int8_t lowPass, int8_t bandPass, int8_t highPass, int8_t balance){
    audioTxMessage.cmd = SETTONE;
    audioTxMessage.value1 = (uint8_t)highPass;
    audioTxMessage.value1 <<= 8;
    audioTxMessage.value1 += (uint8_t)bandPass;
    audioTxMessage.value1 <<= 8;
    audioTxMessage.value1 += (uint8_t)lowPass;
    audioTxMessage.value2 = (uint8_t)balance;
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

uint32_t audioInbuffSize(){
    audioTxMessage.cmd = INBUFF_SIZE;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}


boolean audioIsRunning(){
    audioTxMessage.cmd = ISRUNNING;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

uint32_t audioGetStackHighWatermark(){
    audioTxMessage.cmd = HIGHWATERMARK;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

boolean audioPauseResume(){
    audioTxMessage.cmd = PAUSERESUME;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}
void audioConnectionTimeout(uint32_t timeout_ms, uint32_t timeout_ms_ssl){
    audioTxMessage.cmd = CONNECTION_TIMEOUT;
    audioTxMessage.value1 = timeout_ms;
    audioTxMessage.value2 = timeout_ms_ssl;
    audioMessage RX = transmitReceive(audioTxMessage);
    (void)RX;
}

uint32_t audioGetFileSize(){
    audioTxMessage.cmd = GET_FILESIZE;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

uint32_t audioGetFilePosition(){
    audioTxMessage.cmd = GET_FILEPOSITION;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

uint16_t audioGetVUlevel(){
    audioTxMessage.cmd = GET_VULEVEL;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

uint32_t audioGetFileDuration(){
    audioTxMessage.cmd = GET_AUDIOFILEDURATION;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

uint32_t audioGetCurrentTime(){
    audioTxMessage.cmd = GET_AUDIOCURRENTTIME;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}