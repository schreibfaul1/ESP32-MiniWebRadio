/*
 * a2dp_sink.h
 *
 *  Created on: 26.08.2020
 *  Updated on: 27.10.2023
 *      Author: Wolle
 */

#ifndef A2DP_SINK_H_
#define A2DP_SINK_H_

#include <Arduino.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_avrc_api.h"
#include "esp_a2dp_api.h"
#include "esp_spp_api.h"
#include "driver/i2s.h"
#include "esp_avrc_api.h"
#include <esp32-hal-log.h>

#define APP_SIG_WORK_DISPATCH (0x01)

/* AVRCP used transaction labels */
#define APP_RC_CT_TL_GET_CAPS            (0)
#define APP_RC_CT_TL_GET_META_DATA       (1)
#define APP_RC_CT_TL_RN_TRACK_CHANGE     (2)
#define APP_RC_CT_TL_RN_PLAYBACK_CHANGE  (3)
#define APP_RC_CT_TL_RN_PLAY_POS_CHANGE  (4)

extern __attribute__((weak)) void bt_info(const char*);
extern __attribute__((weak)) void bt_state(const char*);
extern __attribute__((weak)) void bt_rssi(int8_t);
extern __attribute__((weak)) void bt_metadata(const char* md, uint8_t id);

typedef void (* app_callback_t) (uint16_t event, void *param);

typedef struct {
    uint16_t             sig;      /*!< signal to app_task */
    uint16_t             event;    /*!< message event id */
    app_callback_t       cb;       /*!< context switch callback */
    void                 *param;   /*!< parameter area needs to be last */
} app_msg_t;

enum {
    BT_APP_EVT_STACK_UP = 0,
};

void                  config_i2s(uint16_t buf_count, uint16_t buf_len);
void                  set_i2s_pinout(int8_t BCLK, int8_t LRC, int8_t DOUT);
void                  bt_set_volume(uint8_t vol);
esp_a2d_audio_state_t get_audio_state();
esp_a2d_mct_t         get_audio_type();
bool                  bt_app_work_dispatch(app_callback_t p_cback, uint16_t event, void* p_params, int param_len);
void                  bt_app_work_dispatched(app_msg_t* msg);
bool                  bt_app_send_msg(app_msg_t* msg);
void                  bt_app_task_handler(void* arg);
void                  bt_app_task_start_up(void);
void                  bt_app_task_shut_down(void);
void                  bt_app_alloc_meta_buffer(esp_avrc_ct_cb_param_t* param);
void                  bt_app_rc_ct_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t* param);
void                  bt_app_rc_tg_cb(esp_avrc_tg_cb_event_t event, esp_avrc_tg_cb_param_t* param);
void                  bt_av_hdl_a2d_evt(uint16_t event, void* p_param);
void                  bt_av_new_track();
void                  bt_av_previous_track();
void                  bt_av_next_track();
void                  bt_av_pause_track();
void                  bt_av_resume_track();
void                  bt_av_get_last_RSSI_delta();
void                  bt_av_notify_evt_handler(uint8_t event_id, esp_avrc_rn_param_t* event_parameter);
void                  bt_av_hdl_avrc_evt(uint16_t event, void* p_param);
void                  bt_av_hdl_stack_evt(uint16_t event, void* p_param);
void                  bt_app_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t* param);
void                  bt_app_a2d_data_cb(const uint8_t* data, uint32_t len);
bool                  a2dp_sink_deinit();
bool                  a2dp_sink_init(String deviceName, int8_t BCLK, int8_t LRC, int8_t DOUT);

#endif /* A2DP_SINK_H_ */
