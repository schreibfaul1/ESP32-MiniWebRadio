/*
 * a2dp_sink.cpp.c
 *
 *  Created on: 27.08.2020
 *  Updated on: 27.10.2023
 *      Author: Wolle
 */

#include "a2dp_sink.h"

xQueueHandle               s_bt_app_task_queue = NULL;
xTaskHandle                s_bt_app_task_handle = NULL;
esp_a2d_audio_state_t      s_audio_state = ESP_A2D_AUDIO_STATE_STOPPED;
const char*                s_a2d_conn_state_str[4] = {"Disconnected", "Connecting", "Connected", "Disconnecting"};
const char*                s_a2d_audio_state_str[3] = {"Suspended", "Stopped", "Started"};
esp_a2d_cb_param_t        *s_a2d = NULL;
esp_a2d_mct_t              s_audio_type = 0;
uint8_t*                   s_bda = NULL;
String                     s_BT_sink_name = "";
i2s_port_t                 s_i2s_port = I2S_NUM_0;
i2s_config_t               s_i2s_config;
i2s_pin_config_t           s_pin_config;
char*                      s_chbuf = NULL;
boolean                    s_f_a2dp_sink_active = false;
uint8_t                    s_vol = 64;
int8_t                     s_BCLK = 27;
int8_t                     s_LRC = 26;
int8_t                     s_DOUT = 25;
esp_spp_mode_t             s_esp_spp_mode = ESP_SPP_MODE_CB;
esp_avrc_rn_evt_cap_mask_t s_avrc_peer_rn_cap_set = {0};
esp_avrc_rn_evt_cap_mask_t s_avrc_peer_rn_cap = {0};

esp_a2d_cb_param_t::a2d_conn_stat_param s_a2d_conn_stat;
//----------------------------------------------------------------------------------------------------------------------------------------------------
void config_i2s(uint16_t buf_count, uint16_t buf_len) {

    // setup default i2s config
    s_i2s_config.mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX);
    s_i2s_config.sample_rate = 44100;
    s_i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
    s_i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    s_i2s_config.communication_format = (i2s_comm_format_t) (I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB);
    s_i2s_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1; // interrupt priority
    s_i2s_config.dma_buf_count = 8;
    s_i2s_config.dma_buf_len = 64;
    s_i2s_config.tx_desc_auto_clear = true;
    s_i2s_config.use_apll = false;

    i2s_driver_install(s_i2s_port, &s_i2s_config, 0, NULL);

    // setup default pins
    s_pin_config.bck_io_num = s_BCLK;                // BCLK
    s_pin_config.ws_io_num = s_LRC;                  // LRC
    s_pin_config.data_out_num = s_DOUT;              // DOUT
    s_pin_config.data_in_num = I2S_PIN_NO_CHANGE; // DIN

    i2s_set_pin(s_i2s_port, &s_pin_config);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void set_i2s_pinout(int8_t BCLK, int8_t LRC, int8_t DOUT){ // overwrite default pins

    s_pin_config.bck_io_num   = BCLK;              // BCLK
    s_pin_config.ws_io_num    = LRC;               // LRC
    s_pin_config.data_out_num = DOUT;              // DOUT
    s_pin_config.data_in_num  = I2S_PIN_NO_CHANGE; // DIN

    i2s_set_pin(s_i2s_port, &s_pin_config);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_set_volume(uint8_t vol){
    s_vol = vol * 3;  // 0...21 --> 0...63
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
esp_a2d_audio_state_t get_audio_state() {
    return s_audio_state;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
esp_a2d_mct_t get_audio_type() {
    return s_audio_type;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool bt_app_work_dispatch(app_callback_t p_cback, uint16_t event, void *p_params, int param_len){

    if(bt_info){
        sprintf(s_chbuf, "event 0x%x, param len %d", event, param_len);
        bt_info(s_chbuf);
    }

    app_msg_t msg;
    memset(&msg, 0, sizeof(app_msg_t));

    msg.sig = APP_SIG_WORK_DISPATCH;
    msg.event = event;
    msg.cb = p_cback;

    if (param_len == 0) {
        return bt_app_send_msg(&msg);
    } else if (p_params && param_len > 0) {
        if ((msg.param = malloc(param_len)) != NULL) {
            memcpy(msg.param, p_params, param_len);
            return bt_app_send_msg(&msg);
        }
    }
    return false;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_app_work_dispatched(app_msg_t *msg){
    if(bt_info){
        sprintf(s_chbuf, "event 0x%x, sig 0x%x dispatched", msg->event, msg->sig);
        bt_info(s_chbuf);
    }
    if (msg->cb) {
        msg->cb(msg->event, msg->param);
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool bt_app_send_msg(app_msg_t *msg){
    if(bt_info){
        sprintf(s_chbuf, "send msg: event 0x%x, sig 0x%x", msg->event, msg->sig);
        bt_info(s_chbuf);
    }
    if (msg == NULL) {
        return false;
    }

    if (xQueueSend(s_bt_app_task_queue, msg, 10 / portTICK_RATE_MS) != pdTRUE) {
        log_e("xQueue send failed");
        return false;
    }
    return true;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_app_task_handler(void *arg){
    app_msg_t msg;
    for (;;) {
        if(pdTRUE == xQueueReceive(s_bt_app_task_queue, &msg, (portTickType)portMAX_DELAY)){
            switch (msg.sig) {
                case APP_SIG_WORK_DISPATCH:
                    if(bt_info){
                        sprintf(s_chbuf, "task handler: APP_SIG_WORK_DISPATCH sig: %u", msg.sig);
                        bt_info(s_chbuf);
                    }
                    bt_app_work_dispatched(&msg);
                    break;
                default:
                    if(bt_info){
                        sprintf(s_chbuf, "task handler: unhandled sig: 0%x", msg.sig);
                        bt_info(s_chbuf);
                    }
                    break;
            } // switch (msg.sig)

            if (msg.param) {
                free(msg.param);
            }
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_app_task_start_up(void){
    s_bt_app_task_queue = xQueueCreate(10, sizeof(app_msg_t));
    xTaskCreate(bt_app_task_handler, "BtAppT", 2048, NULL, configMAX_PRIORITIES - 3, &s_bt_app_task_handle);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void  bt_app_task_shut_down(void){
    if (s_bt_app_task_handle) {
        vTaskDelete(s_bt_app_task_handle);
        s_bt_app_task_handle = NULL;
    }
    if (s_bt_app_task_queue) {
        vQueueDelete(s_bt_app_task_queue);
        s_bt_app_task_queue = NULL;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_app_alloc_meta_buffer(esp_avrc_ct_cb_param_t *param){
    esp_avrc_ct_cb_param_t *rc = (esp_avrc_ct_cb_param_t *)(param);
    uint8_t *attr_text = (uint8_t *) malloc (rc->meta_rsp.attr_length + 1);
    memcpy(attr_text, rc->meta_rsp.attr_text, rc->meta_rsp.attr_length);
    attr_text[rc->meta_rsp.attr_length] = 0;
    if(bt_info){
        sprintf(s_chbuf, "metadata: attr_text= %s", attr_text);
        bt_info(s_chbuf);
    }
    rc->meta_rsp.attr_text = attr_text;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_app_rc_ct_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param){
    switch (event) {
        case ESP_AVRC_CT_METADATA_RSP_EVT:
            if(bt_info){
                bt_info("ESP_AVRC_CT_METADATA_RSP_EVT");
            }
            bt_app_alloc_meta_buffer(param);
            bt_app_work_dispatch(bt_av_hdl_avrc_evt, event, param, sizeof(esp_avrc_ct_cb_param_t));
            break;
        case ESP_AVRC_CT_CONNECTION_STATE_EVT:
            if(bt_info){
                bt_info("ESP_AVRC_CT_CONNECTION_STATE_EVT");
            }
            bt_app_work_dispatch(bt_av_hdl_avrc_evt, event, param, sizeof(esp_avrc_ct_cb_param_t));
            break;
        case ESP_AVRC_CT_PASSTHROUGH_RSP_EVT:
            if(bt_info){
                bt_info("ESP_AVRC_CT_PASSTHROUGH_RSP_EVT");
            }
            bt_app_work_dispatch(bt_av_hdl_avrc_evt, event, param, sizeof(esp_avrc_ct_cb_param_t));
            break;
        case ESP_AVRC_CT_CHANGE_NOTIFY_EVT:
            if(bt_info){
                bt_info("ESP_AVRC_CT_CHANGE_NOTIFY_EVT");
            }
            bt_app_work_dispatch(bt_av_hdl_avrc_evt, event, param, sizeof(esp_avrc_ct_cb_param_t));
            break;
        case ESP_AVRC_CT_REMOTE_FEATURES_EVT: {
            if(bt_info){
                bt_info("ESP_AVRC_CT_REMOTE_FEATURES_EVT");
            }
            bt_app_work_dispatch(bt_av_hdl_avrc_evt, event, param, sizeof(esp_avrc_ct_cb_param_t));
            break;
        case ESP_AVRC_CT_GET_RN_CAPABILITIES_RSP_EVT: {
            bt_app_work_dispatch(bt_av_hdl_avrc_evt, event, param, sizeof(esp_avrc_ct_cb_param_t));
            break;
        }
        }
        default:
            log_e("Invalid AVRC event: %d", event);
            break;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_app_rc_tg_cb(esp_avrc_tg_cb_event_t event, esp_avrc_tg_cb_param_t *param){
    ESP_LOGD(BT_AV_TAG, "%s", __func__);
    switch (event) {
        case ESP_AVRC_TG_CONNECTION_STATE_EVT:
        case ESP_AVRC_TG_REMOTE_FEATURES_EVT:
        case ESP_AVRC_TG_PASSTHROUGH_CMD_EVT:
        case ESP_AVRC_TG_SET_ABSOLUTE_VOLUME_CMD_EVT:
        case ESP_AVRC_TG_REGISTER_NOTIFICATION_EVT:
        case ESP_AVRC_TG_SET_PLAYER_APP_VALUE_EVT:{
        //    app_work_dispatch(ccall_av_hdl_avrc_tg_evt, event, param, sizeof(esp_avrc_tg_cb_param_t));
            break;
        }
        default:
            log_e("Unsupported AVRC event: %d", event);
            break;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_av_hdl_a2d_evt(uint16_t event, void *p_param){
    switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT: {
        s_a2d = (esp_a2d_cb_param_t *)(p_param);
        if(bt_info){
            sprintf(s_chbuf, "ESP_A2D_CONNECTION_STATE_EVT %i", event);
            bt_info(s_chbuf);
        }
        s_bda = s_a2d->conn_stat.remote_bda;
        if(bt_state){
            sprintf(s_chbuf, "A2DP connection state: %s", s_a2d_conn_state_str[s_a2d->conn_stat.state]);
            bt_state(s_chbuf);
        }
        if(s_a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED){
            // esp_err_t esp_err = esp_bt_gap_read_remote_name(s_a2d->conn_stat.remote_bda);
            // if (esp_err!=ESP_OK){
            //     log_w("esp_bt_gap_read_remote_name");
            // }
            s_a2d_conn_stat = s_a2d->conn_stat;
            esp_bt_gap_read_rssi_delta(s_a2d_conn_stat.remote_bda);
        }
        break;
    }
    case ESP_A2D_AUDIO_STATE_EVT: {
        s_a2d = (esp_a2d_cb_param_t *)(p_param);
        if(bt_state){
            sprintf(s_chbuf, "A2DP audio state: %s", s_a2d_audio_state_str[s_a2d->audio_stat.state]);
            bt_state(s_chbuf);
        }
        s_audio_state = s_a2d->audio_stat.state;
        if (ESP_A2D_AUDIO_STATE_STARTED == s_a2d->audio_stat.state) {
            bt_av_new_track();
        }
        break;
    }
    case ESP_A2D_AUDIO_CFG_EVT: {
        esp_a2d_cb_param_t *esp_a2d_callback_param = (esp_a2d_cb_param_t *)(p_param);
        s_audio_type = esp_a2d_callback_param->audio_cfg.mcc.type;
        s_a2d = (esp_a2d_cb_param_t *)(p_param);
        // for now only SBC stream is supported
        if (s_a2d->audio_cfg.mcc.type == ESP_A2D_MCT_SBC) {
            s_i2s_config.sample_rate = 16000;
            char oct0 = s_a2d->audio_cfg.mcc.cie.sbc[0];
            if (oct0 & (0x01 << 6)) {
                s_i2s_config.sample_rate = 32000;
                if(bt_info) {bt_info("SampleRate: 32000 Hz");}
            } else if (oct0 & (0x01 << 5)) {
                s_i2s_config.sample_rate = 44100;
                if(bt_info) {bt_info("SampleRate: 44100 Hz");}
            } else if (oct0 & (0x01 << 4)) {
                s_i2s_config.sample_rate = 48000;
                if(bt_info) {bt_info("SampleRate: 48000 Hz");}
            }
            i2s_set_clk(s_i2s_port, s_i2s_config.sample_rate, s_i2s_config.bits_per_sample, (i2s_channel_t)2);
            if(bt_info){
                sprintf(s_chbuf, "configure audio player [%02x-%02x-%02x-%02x]", s_a2d->audio_cfg.mcc.cie.sbc[0], s_a2d->audio_cfg.mcc.cie.sbc[1],
                                                                                 s_a2d->audio_cfg.mcc.cie.sbc[2], s_a2d->audio_cfg.mcc.cie.sbc[3]);
                bt_info(s_chbuf);
            }
        }
        break;
    }
    default:
        log_e("unhandled evt 0x%x", event);
        break;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_av_new_track(){
    if(bt_state){bt_state("new track");}
    //Register notifications and request metadata
    esp_avrc_ct_send_metadata_cmd(0, ESP_AVRC_MD_ATTR_TITLE | ESP_AVRC_MD_ATTR_ARTIST | ESP_AVRC_MD_ATTR_ALBUM | ESP_AVRC_MD_ATTR_GENRE);
    esp_avrc_ct_send_register_notification_cmd(1, ESP_AVRC_RN_TRACK_CHANGE, 0);
    esp_avrc_ct_send_register_notification_cmd(2, ESP_AVRC_RN_NOW_PLAYING_CHANGE,  0);
    esp_avrc_ct_send_register_notification_cmd(3, ESP_AVRC_RN_PLAY_POS_CHANGED, 0);
    esp_avrc_ct_send_register_notification_cmd(4, ESP_AVRC_RN_VOLUME_CHANGE,  0);
 }
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_av_previous_track(){
    esp_avrc_ct_send_passthrough_cmd(1, ESP_AVRC_PT_CMD_BACKWARD, ESP_AVRC_PT_CMD_STATE_PRESSED);
    vTaskDelay(300/portTICK_PERIOD_MS);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_av_next_track(){
    esp_avrc_ct_send_passthrough_cmd(1, ESP_AVRC_PT_CMD_FORWARD, ESP_AVRC_PT_CMD_STATE_PRESSED);
    vTaskDelay(300/portTICK_PERIOD_MS);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_av_pause_track(){
    esp_avrc_ct_send_passthrough_cmd(1, ESP_AVRC_PT_CMD_PAUSE, ESP_AVRC_PT_CMD_STATE_PRESSED);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_av_resume_track(){
    esp_avrc_ct_send_passthrough_cmd(1, ESP_AVRC_PT_CMD_PLAY, ESP_AVRC_PT_CMD_STATE_PRESSED);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_av_get_last_RSSI_delta(){
    if(!s_f_a2dp_sink_active) return;
    if(s_a2d_conn_stat.remote_bda){esp_bt_gap_read_rssi_delta(s_a2d_conn_stat.remote_bda);}
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_av_notify_evt_handler(uint8_t event_id, esp_avrc_rn_param_t* event_parameter){
    switch (event_id) {
        case ESP_AVRC_RN_TRACK_CHANGE:
            if(bt_info){
                sprintf(s_chbuf, "ESP_AVRC_RN_TRACK_CHANGE %d",event_id);
                bt_info(s_chbuf);
            }
            bt_av_new_track();
            break;
        case ESP_AVRC_RN_NOW_PLAYING_CHANGE:
            if(bt_info){
                sprintf(s_chbuf, "ESP_AVRC_RN_NOW_PLAYING_CHANGE %d",event_id);
                bt_info(s_chbuf);
            }
            break;
        case ESP_AVRC_RN_PLAY_STATUS_CHANGE:
            if(bt_info){
                sprintf(s_chbuf, "ESP_AVRC_RN_PLAY_STATUS_CHANGE %d, param %d",event_id, event_parameter->playback);
                bt_info(s_chbuf);
            }

            break;
        case ESP_AVRC_RN_PLAY_POS_CHANGED:
            if(bt_info){
                sprintf(s_chbuf, "Play position changed: %d-ms", event_parameter->play_pos);
                bt_info(s_chbuf);
            }
            break;
        case ESP_AVRC_RN_VOLUME_CHANGE:
            if(bt_state){
                sprintf(s_chbuf, "Volume changed: %d", event_parameter->volume);
                bt_state(s_chbuf);
            }
        default:
            log_e("unhandled evt %d", event_id);
            break;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_av_hdl_avrc_evt(uint16_t event, void *p_param){ // Audio Video Remote Control

    esp_avrc_ct_cb_param_t *rc = (esp_avrc_ct_cb_param_t *)(p_param);
    switch (event) {
        case ESP_AVRC_CT_CONNECTION_STATE_EVT: {
            uint8_t *bda = rc->conn_stat.remote_bda;
            s_bda = rc->conn_stat.remote_bda;
            if(bt_info){
                sprintf(s_chbuf, "AVRC conn_state evt: state %d, [%02x:%02x:%02x:%02x:%02x:%02x]",
                                                                  rc->conn_stat.connected, bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
                bt_info(s_chbuf);
            }
            if (rc->conn_stat.connected) {
                esp_avrc_ct_send_get_rn_capabilities_cmd(APP_RC_CT_TL_GET_CAPS);
            //    esp_avrc_ct_send_register_notification_cmd(1, ESP_AVRC_RN_PLAY_STATUS_CHANGE, 0);
            }
            break;
        }
        case ESP_AVRC_CT_PASSTHROUGH_RSP_EVT: {
            if(bt_info){
                sprintf(s_chbuf, "AVRC passthrough rsp: key_code 0x%x, key_state %u", rc->psth_rsp.key_code, rc->psth_rsp.key_state);
                bt_info(s_chbuf);
            }
            break;
        }
        case ESP_AVRC_CT_METADATA_RSP_EVT: {
            if(bt_info){
                sprintf(s_chbuf, "AVRC metadata rsp: attribute id 0x%x, %s", (uint32_t)rc->meta_rsp.attr_id, rc->meta_rsp.attr_text);
                bt_info(s_chbuf);
            }
            if(bt_metadata){bt_metadata((const char*)rc->meta_rsp.attr_text, rc->meta_rsp.attr_id);}
            free(rc->meta_rsp.attr_text);
            break;
        }
        case ESP_AVRC_CT_CHANGE_NOTIFY_EVT: {
            if(bt_info){
                sprintf(s_chbuf, "AVRC event notification: %u", rc->change_ntf.event_id); //, rc->change_ntf.event_parameter);
                bt_info(s_chbuf);
            }
            bt_av_notify_evt_handler(rc->change_ntf.event_id, &rc->change_ntf.event_parameter);
            break;
        }
        case ESP_AVRC_CT_REMOTE_FEATURES_EVT: {
            if(bt_info){
                sprintf(s_chbuf, "AVRC remote features 0x%x", rc->rmt_feats.feat_mask);
                bt_info(s_chbuf);
            }
            break;
        }
        case ESP_AVRC_CT_GET_RN_CAPABILITIES_RSP_EVT: {
            if(bt_info){
                sprintf(s_chbuf, "remote rn_cap: count %d, bitmask 0x%x", rc->get_rn_caps_rsp.cap_count, rc->get_rn_caps_rsp.evt_set.bits);
                bt_info(s_chbuf);
            }
            s_avrc_peer_rn_cap.bits = rc->get_rn_caps_rsp.evt_set.bits;
            break;
        }
        case ESP_AVRC_CT_SET_ABSOLUTE_VOLUME_RSP_EVT:{ /*!< set absolute volume response event */} break;
        default:
            log_e("unhandled evt %d", event);
            break;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_av_hdl_stack_evt(uint16_t event, void *p_param){
    esp_err_t res;
    switch (event) {
        case BT_APP_EVT_STACK_UP: {
            if(bt_info){
                sprintf(s_chbuf, "av_hdl_stack_evt %s","BT_APP_EVT_STACK_UP");
                bt_info(s_chbuf);
            }
            /* set up device name */
            res = esp_bt_dev_set_device_name(s_BT_sink_name.c_str());
            if(res != ESP_OK){
                log_e("esp_bt_dev_set_device_name error");
            }
            /* initialize A2DP sink */
            res = esp_a2d_register_callback(bt_app_a2d_cb);
            if(res != ESP_OK){
                log_e("esp_a2d_register_callback error");
            }

            res = esp_a2d_sink_register_data_callback(bt_app_a2d_data_cb);
            if(res != ESP_OK){
                log_e("esp_a2d_sink_register_data_callback error");
            }
            res = esp_a2d_sink_init();
            if(res != ESP_OK){
                if(res == ESP_ERR_INVALID_STATE){log_e("esp_a2d_sink_init invalid state");}
                else {log_e("esp_a2d_sink_init error");}
            }
            /* set discoverable and connectable mode, wait to be connected */
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            break;
        }
        default:
            log_e("unhandled evt %d",event);
            break;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_app_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param){
    switch (event) {
        case ESP_A2D_CONNECTION_STATE_EVT:
            if(bt_info){
                sprintf(s_chbuf, "ESP_A2D_CONNECTION_STATE_EVT");
                bt_info(s_chbuf);
            }
            bt_app_work_dispatch(bt_av_hdl_a2d_evt, event, param, sizeof(esp_a2d_cb_param_t));
            break;
        case ESP_A2D_AUDIO_STATE_EVT:
            if(bt_info){
                sprintf(s_chbuf, "ESP_A2D_AUDIO_STATE_EVT");
                bt_info(s_chbuf);
            }
            s_audio_state = param->audio_stat.state;
            bt_app_work_dispatch(bt_av_hdl_a2d_evt,event, param, sizeof(esp_a2d_cb_param_t));
        //    bt_av_new_track();
            break;
        case ESP_A2D_AUDIO_CFG_EVT: {

            bt_app_work_dispatch(bt_av_hdl_a2d_evt, event, param, sizeof(esp_a2d_cb_param_t));
            break;
        }
        case ESP_A2D_PROF_STATE_EVT:{ // indicate a2dp init&deinit complete
            if(bt_info){
                sprintf(s_chbuf, "ESP_A2D_PROF_STATE_EVT");
                bt_info(s_chbuf);
            }
            break;
        }

        default:
            log_e("Invalid A2DP event: %d", event);
            break;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_app_a2d_data_cb(const uint8_t *data, uint32_t len) {

    uint32_t s32;
    int16_t s16_1;
    int16_t s16_2;

    size_t i2s_bytes_written = 0;

    for(uint32_t i = 0; i < len; i += 4){

        s16_1 = data[i + 0] + (data[i + 1] << 8);
        s16_2 = data[i + 2] + (data[i + 3] << 8);

        s16_1 *=((float)s_vol / 64);
        s16_2 *=((float)s_vol / 64);

        s32 = s16_1 + (s16_2 << 16);

        size_t bytesWritten = 0;
        if(i2s_write(s_i2s_port,(const char*) &s32, sizeof(uint32_t), &bytesWritten, portMAX_DELAY) != ESP_OK){
            log_e("i2s_write has failed");
        }
        i2s_bytes_written += bytesWritten;
    }
    if (i2s_bytes_written < len){
       log_e("Timeout: not all bytes were written to I2S");
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void bt_app_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param){
    switch (event) {
        case ESP_BT_GAP_DISC_RES_EVT:{ /*!< Device discovery result event */} break;
        case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:{ /*!< Discovery state changed event */} break;
        case ESP_BT_GAP_RMT_SRVCS_EVT:{  /*!< Get remote services event */ } break;
        case ESP_BT_GAP_RMT_SRVC_REC_EVT:{/*!< Get remote service record event */} break;
        case ESP_BT_GAP_AUTH_CMPL_EVT: {
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {log_i("authentication success: %s", param->auth_cmpl.device_name);}
            else {                                               log_i("authentication failed, status:%d", param->auth_cmpl.stat);}
            break;
        }
        case ESP_BT_GAP_PIN_REQ_EVT: {
            //    memcpy(peer_bd_addr, param->pin_req.bda, ESP_BD_ADDR_LEN);
            //    log_i("partner address: %s", to_str(peer_bd_addr));
            }
            break;
        case ESP_BT_GAP_CFM_REQ_EVT: {
            //    memcpy(peer_bd_addr, param->cfm_req.bda, ESP_BD_ADDR_LEN);
            //    log_i("partner address: %s", to_str(peer_bd_addr));
            //    log_i("ESP_BT_GAP_CFM_REQ_EVT Please confirm the passkey: %d", param->cfm_req.num_val);
            //    pin_code_int = param->key_notif.passkey;
            //    pin_code_request = Confirm;
            }
            break;
        case ESP_BT_GAP_KEY_NOTIF_EVT: {
            //    log_i("ESP_BT_GAP_KEY_NOTIF_EVT passkey:%d", param->key_notif.passkey);
            //    pin_code_int = param->key_notif.passkey;
            //    pin_code_request = Reply;
            }
            break;
        case ESP_BT_GAP_KEY_REQ_EVT: {
            //    log_i("ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
            //    pin_code_request = Reply;
            }
            break;
        case ESP_BT_GAP_READ_RSSI_DELTA_EVT: {
            esp_bt_gap_cb_param_t::read_rssi_delta_param last_rssi_delta;
            last_rssi_delta = param->read_rssi_delta;
            // if(bt_info){
            //     sprintf(s_chbuf, "last rssi delta %i", last_rssi_delta.rssi_delta);
            //     bt_info(s_chbuf);
            // }
            if(bt_rssi){bt_rssi(last_rssi_delta.rssi_delta);}
            }
            break;
        case ESP_BT_GAP_CONFIG_EIR_DATA_EVT:{  /*!< Config EIR data event */} break;
        case ESP_BT_GAP_SET_AFH_CHANNELS_EVT:{/*!< Set AFH channels event */} break;
        case ESP_BT_GAP_READ_REMOTE_NAME_EVT: {
            // log_i("ESP_BT_GAP_READ_REMOTE_NAME_EVT stat:%d", param->read_rmt_name.stat);
            if (param->read_rmt_name.stat == ESP_BT_STATUS_SUCCESS ) {
                if(bt_info){
                    sprintf(s_chbuf, "remote name: %s", param->read_rmt_name.rmt_name);
                    bt_info(s_chbuf);
                }
            }
            }
            break;
        case ESP_BT_GAP_MODE_CHG_EVT: {
            //    log_i("ESP_BT_GAP_MODE_CHG_EVT");
           }
           break;
        case ESP_BT_GAP_REMOVE_BOND_DEV_COMPLETE_EVT:{/*!< remove bond device complete event */} break;
        case ESP_BT_GAP_QOS_CMPL_EVT:{ /*!< QOS complete event */} break;
        case ESP_BT_GAP_ACL_CONN_CMPL_STAT_EVT:{ /*!< ACL connection complete status event */} break;
        case ESP_BT_GAP_EVT_MAX:{} break;
        default: {
            log_e("unknown event: %d", event);
            break;
        }
    }
    return;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool a2dp_sink_deinit(){
    esp_err_t res;
    s_f_a2dp_sink_active = false;

    res = esp_a2d_sink_deinit();
    if(res != ESP_OK){log_e("a2dp sink deinit failed"); goto exit;}
    else{
        if(bt_info){
            sprintf(s_chbuf, "avrc deinit okay");
            bt_info(s_chbuf);
        }
    }

    res = esp_avrc_ct_deinit();
    if(res != ESP_OK){log_e("avrc deinit failed"); goto exit;}
    else{
        if(bt_info){
            sprintf(s_chbuf, "avrc deinit okay");
            bt_info(s_chbuf);
        }
    }

    res = esp_bluedroid_disable();
    if(res != ESP_OK){log_e("Failed to disable bluedroid"); goto exit;}
    else{
        if(bt_info){
            sprintf(s_chbuf, "bluedroid disabled");
            bt_info(s_chbuf);
        }
    }

    res = esp_bluedroid_deinit();
    if(res !=  ESP_OK){if(bt_info) bt_info("Failed to deinit bluedroid"); goto exit;}
    else{
        if(bt_info){
            sprintf(s_chbuf, "bluedroid deinit okay");
            bt_info(s_chbuf);
        }
    }

    res = i2s_driver_uninstall(s_i2s_port);
    if(res != ESP_OK){if(bt_info) bt_info("Failed to uninstall i2s"); goto exit;}

    bt_app_task_shut_down();

    res = esp_bt_controller_disable();
    if(res != ESP_OK){if(bt_info) bt_info("Failed to disable bt controller");}
    else{
        if(bt_info){
            sprintf(s_chbuf, "bt controller disable okay");
            bt_info(s_chbuf);
        }
    }

    // waiting for status change
    while(esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_ENABLED) {delay(50);}

    if(esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_INITED){
        res = esp_bt_controller_deinit();
        if(res != ESP_OK){if(bt_info) bt_info("Failed to deinit bt controller");}
        else{
            if(bt_info){
                sprintf(s_chbuf, "bt controller deinit okay");
                bt_info(s_chbuf);
            }
        }
    }
    res = esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
    if(res != ESP_OK){if(bt_info) bt_info("esp_bt_controller_mem_release failed");}
    else{
        if(bt_info){
            sprintf(s_chbuf, "esp_bt_controller_mem_release okay");
            bt_info(s_chbuf);
        }
    }

    if(bt_state) bt_state("BT disabled");
    if(s_chbuf){free(s_chbuf); s_chbuf = NULL;}
    s_a2d = NULL;
    *s_a2d_conn_stat.remote_bda = 0;
    config_i2s(16, 512);
    return true;

exit:
    if(s_chbuf){free(s_chbuf); s_chbuf = NULL;}
    return false;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool a2dp_sink_init(String deviceName, int8_t BCLK, int8_t LRC, int8_t DOUT){

    s_BCLK = BCLK;
    s_LRC  = LRC;
    s_DOUT = DOUT;

    i2s_driver_uninstall((i2s_port_t)0);
    s_a2d = NULL;
    *s_a2d_conn_stat.remote_bda = 0;

    esp_err_t res;
    if(s_chbuf){free(s_chbuf); s_chbuf = NULL;}
    s_chbuf = (char*)malloc(512);
    if(!s_chbuf) log_e("oom");
    s_BT_sink_name = deviceName;

    if(bt_info){
        sprintf(s_chbuf, "Device name will be set to '%s'", s_BT_sink_name.c_str());
        bt_info(s_chbuf);
    }
    if(!btStart()) {log_e("Failed to initialize controller"); return false;}
    else{
        if(bt_info){
            sprintf(s_chbuf, "bt controller initialized");
            bt_info(s_chbuf);
        }
    }

    esp_bluedroid_status_t bt_stack_status = esp_bluedroid_get_status();
    if(bt_stack_status == ESP_BLUEDROID_STATUS_UNINITIALIZED){
        res = esp_bluedroid_init();
        if(res != ESP_OK) {log_e("Failed to initialize bluedroid"); return false;}
        else{
            if(bt_info){
                sprintf(s_chbuf, "bluedroid initialized");
                bt_info(s_chbuf);
            }
        }
    }
    res = esp_bluedroid_enable();
    if(res != ESP_OK) {log_e("Failed to enable bluedroid"); return false;}
    else{
        if(bt_info){
            sprintf(s_chbuf, "bluedroid enabled");
            bt_info(s_chbuf);
        }
    }
//  res = esp_spp_init(s_esp_spp_mode); // disabled in menuconfig
    res = esp_bt_gap_register_callback(bt_app_gap_cb);

    /* initialize AVRCP controller */
    esp_avrc_ct_init();
    res = esp_avrc_ct_register_callback(bt_app_rc_ct_cb);
    if(res != ESP_OK){log_e("AVRCP controller not initialized!");}

    /* initialize AVRCP target */ // not used yet
    //res = esp_avrc_tg_init();
    //if(res == ESP_OK){
    //    esp_avrc_tg_register_callback(bt_app_rc_tg_cb);
    //     // add request to ESP_AVRC_RN_VOLUME_CHANGE
    //    esp_avrc_rn_evt_bit_mask_operation(ESP_AVRC_BIT_MASK_OP_SET, &s_avrc_peer_rn_cap_set, ESP_AVRC_RN_VOLUME_CHANGE);
    //     if(esp_avrc_tg_set_rn_evt_cap(&s_avrc_peer_rn_cap_set) != ESP_OK){
    //         log_e("esp_avrc_tg_set_rn_evt_cap failed");
    //     }
    //}
    //else{log_e("esp_avrc_tg_init failed");}

    bt_app_task_start_up(); // create application task
    bt_app_work_dispatch(bt_av_hdl_stack_evt, BT_APP_EVT_STACK_UP, NULL, 0);

    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_FIXED; // Set default parameters for Legacy Pairing
    esp_bt_pin_code_t pin_code;                         // Use fixed pin code
    pin_code[0] = '1';
    pin_code[1] = '2';
    pin_code[2] = '3';
    pin_code[3] = '4';
    esp_bt_gap_set_pin(pin_type, 4, pin_code);

    config_i2s(8, 64);
    if(bt_state) bt_state("BT enabled");
    s_f_a2dp_sink_active = true;
    return true;
}
