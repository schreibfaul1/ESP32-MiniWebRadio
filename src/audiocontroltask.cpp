// created: 10.02.2022
// updated: 17.08.2024

#include "Audio.h"     // see my repository at github "ESP32-audioI2S"
#include "common.h"


Audio audio;

extern RTIME rtc;
extern SemaphoreHandle_t  mutex_rtc;

enum : uint8_t { SET_VOLUME, GET_VOLUME, GET_BITRATE, CONNECTTOHOST, CONNECTTOFS, CONNECTTOSPEECH, STOPSONG, SETTONE, INBUFF_FILLED,
                 INBUFF_FREE, INBUFF_SIZE, ISRUNNING, HIGHWATERMARK, GET_CODEC, PAUSERESUME, CONNECTION_TIMEOUT, GET_FILESIZE,
                 GET_FILEPOSITION, GET_VULEVEL, GET_AUDIOFILEDURATION, GET_AUDIOCURRENTTIME, SET_TIMEOFFSET, SET_VOLUME_STEPS, SET_COREID};

struct audioMessage{
    uint8_t     cmd;
    const char* txt1;
    const char* txt2;
    const char* txt3;
    uint32_t    value1;
    uint32_t    value2;
    uint32_t    ret;
} audioTxMessage, audioRxMessage;

uint8_t  t_volume = 0;
uint8_t  t_volSteps = 1;
uint8_t  t_volCurve = 1; // 0 = quadratic, 1 = log
uint32_t t_millis = 0;

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
    audio.setVolume(5, t_volCurve); // 0...volumeSteps

    while(true){
        if(xQueueReceive(audioSetQueue, &audioRxTaskMessage, 1) == pdPASS) {
            if(audioRxTaskMessage.cmd == SET_VOLUME){
                audioTxTaskMessage.cmd = SET_VOLUME;
                t_volume = audioRxTaskMessage.value1;
                t_volCurve= audioRxTaskMessage.value2;
                audioTxTaskMessage.ret = 1;
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == SET_VOLUME_STEPS){
                audioTxTaskMessage.cmd = SET_VOLUME_STEPS;
                audio.setVolumeSteps(audioRxTaskMessage.value1);
                t_volSteps = audioRxTaskMessage.value1 / 20;
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
            else if(audioRxTaskMessage.cmd == CONNECTTOSPEECH){
                audioTxTaskMessage.cmd = CONNECTTOSPEECH;
                audioTxTaskMessage.ret = audio.connecttospeech(audioRxTaskMessage.txt1, audioRxTaskMessage.txt2);
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == GET_VOLUME){
                audioTxTaskMessage.cmd = GET_VOLUME;
                audioTxTaskMessage.ret = audio.getVolume();
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == GET_BITRATE){
                audioTxTaskMessage.cmd = GET_BITRATE;
                audioTxTaskMessage.ret = audio.getBitRate(false);
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
            else if(audioRxTaskMessage.cmd == SET_TIMEOFFSET){
                audioTxTaskMessage.cmd = SET_TIMEOFFSET;
                int16_t timeOffset = (int16_t)(audioRxTaskMessage.value1 & 0xFFFF);
                audioTxTaskMessage.ret = audio.setTimeOffset(timeOffset);
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == SET_VOLUME_STEPS){
                audioTxTaskMessage.cmd = SET_VOLUME_STEPS;
                audio.setVolumeSteps(audioRxTaskMessage.value1);
                audioTxTaskMessage.ret = 1;
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else if(audioRxTaskMessage.cmd == SET_COREID){
                audioTxTaskMessage.cmd = SET_COREID;
                audio.setAudioTaskCore(audioRxTaskMessage.value1);
                audioTxTaskMessage.ret = 1;
                xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
            }
            else{
                SerialPrintfln(ANSI_ESC_RED "Error: unknown audioTaskMessage %i", audioRxTaskMessage.cmd);
            }
        }
        audio.loop();

        if(t_millis + 30 < millis()){
            t_millis = millis();
            uint8_t v = audio.getVolume();
            if (v > t_volume){
                if(v >= t_volume + t_volSteps) {if(v - t_volSteps <   0) audio.setVolume(0, t_volCurve);   else  audio.setVolume(v- t_volSteps, t_volCurve);}
                else audio.setVolume(t_volume, t_volCurve);
            }
            if (v < t_volume){
                if(t_volume + t_volSteps >= v) {if(v + t_volSteps > 255) audio.setVolume(255, t_volCurve); else audio.setVolume(v + t_volSteps, t_volCurve);}
                else audio.setVolume(t_volume, t_volCurve);
            }
        }
        vTaskDelay(7 / portTICK_PERIOD_MS);
    }
}

TaskHandle_t Task1;

void audioInit() {
    xTaskCreatePinnedToCore(
        audioTask,                  /* Function to implement the task */
        "audioplay",                /* Name of the task */
        7500,                       /* Stack size in words */
        NULL,                       /* Task input parameter */
        AUDIOCONTROLTASK_PRIO,      /* Priority of the task */
        &Task1,                     /* Task handle. */
        AUDIOCONTROLTASK_CORE       /* Core where the task should run */
    );
    if(CORE_DEBUG_LEVEL >= 2){
        {SerialPrintfln("audiotask:   is pinned to core " ANSI_ESC_CYAN "%d", AUDIOCONTROLTASK_CORE);}
        {SerialPrintfln("audiotask:   priority is " ANSI_ESC_CYAN "%d", AUDIOCONTROLTASK_PRIO);}
    }
}

void audioControlTaskDelete(){
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

void audioSetVolume(uint8_t vol, uint8_t curve){
    audioTxMessage.cmd = SET_VOLUME;
    audioTxMessage.value1 = vol;
    audioTxMessage.value2 = curve;
    audioMessage RX = transmitReceive(audioTxMessage);
    (void)RX;
}

void audioSetVolumeSteps(uint8_t steps){
    audioTxMessage.cmd = SET_VOLUME_STEPS;
    audioTxMessage.value1 = steps;
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

boolean audioConnecttospeech(const char* text, const char* lang){
    audioTxMessage.cmd = CONNECTTOSPEECH;
    audioTxMessage.txt1 = text;
    audioTxMessage.txt2 = lang;
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

bool audioSetTimeOffset(int16_t timeOffset){
    audioTxMessage.cmd = SET_TIMEOFFSET;
    audioTxMessage.value1 = timeOffset;
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
}

void audioSetCoreID(uint8_t coreId){
    audioTxMessage.cmd = SET_COREID;
    audioTxMessage.value1 = coreId;
    audioMessage RX = transmitReceive(audioTxMessage);
    (void)RX;
}
