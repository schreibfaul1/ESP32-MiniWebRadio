
/*
 * IR.cpp
 *
 *  Created on: 11.08.2017
 *      Author: Wolle
 *  Updated on: 18.07.2023
 */
#include "IR.h"


// global var
DRAM_ATTR int16_t ir_cmd_a = -1; // set from isr
DRAM_ATTR int16_t ir_cmd_b = -1; // set from isr
DRAM_ATTR int16_t ir_adr_a = 0;  // set from isr
DRAM_ATTR int16_t ir_adr_b = 0;  // set from isr
DRAM_ATTR uint8_t ir_rc = 0;
DRAM_ATTR uint8_t ir_addressCode;
DRAM_ATTR uint8_t g_ir_pin;

DRAM_ATTR uint32_t ir_intval_l = 0;
DRAM_ATTR uint32_t ir_intval_h = 0;
DRAM_ATTR int16_t ir_pulsecounter = 0;

IR::IR(uint8_t IR_PIN){
    m_ir_pin = IR_PIN;
    ir_adr_b = -1;
    m_t0 = 0;
    ir_cmd_b = -1;
    m_f_error = false;

    m_ir_buttons[ 0] = 0x00; //
    m_ir_buttons[ 1] = 0x00; //
    m_ir_buttons[ 2] = 0x00; //
    m_ir_buttons[ 3] = 0x00; //
    m_ir_buttons[ 4] = 0x00; //
    m_ir_buttons[ 5] = 0x00; //
    m_ir_buttons[ 6] = 0x00; //
    m_ir_buttons[ 7] = 0x00; //
    m_ir_buttons[ 8] = 0x00; //
    m_ir_buttons[ 9] = 0x00; //
    m_ir_buttons[10] = 0x00; //  mute
    m_ir_buttons[11] = 0x00; //  volume+
    m_ir_buttons[12] = 0x00; //  volume-
    m_ir_buttons[13] = 0x00; //  previous station
    m_ir_buttons[14] = 0x00; //  next station
    m_ir_buttons[15] = 0x00; //
    m_ir_buttons[16] = 0x00; //
    m_ir_buttons[17] = 0x00; //
    m_ir_buttons[18] = 0x00; //
    m_ir_buttons[19] = 0x00; //
}

void IR::begin(){
    if(m_ir_pin >= 0){
        g_ir_pin = m_ir_pin;
        pinMode(m_ir_pin, INPUT_PULLUP);
        attachInterrupt(m_ir_pin, isr_IR, CHANGE); // Interrupts will be handle by isr_IR
    }
}

IR::~IR(){
    ;
}

void IR::set_irButtons(uint8_t btnNr,  uint8_t cmd){
        m_ir_buttons[btnNr] = cmd;
        // log_w("ButtonNumber: %i, Command: 0x%02x", btnNr, cmd);
}

uint8_t* IR::get_irButtons(){
    return m_ir_buttons;
}

void IR::set_irAddress(uint8_t addr){
    ir_addressCode = addr;
}

uint8_t IR::get_irAddress(){
    return ir_addressCode;
}


void IRAM_ATTR IR::setIRresult(uint8_t userCode_a, uint8_t userCode_b, uint8_t dataCode_a, uint8_t dataCode_b){
    ir_cmd_a = dataCode_a;
    ir_cmd_b = dataCode_b;
    ir_adr_a = userCode_a;
    ir_adr_b = userCode_b;
}

void IRAM_ATTR IR::rcCounter (uint8_t rc){
        ir_rc = rc;
}

void IRAM_ATTR IR::error(uint32_t intval_l, uint32_t intval_h, uint8_t pulsecounter){
        ir_intval_l = intval_l;
        ir_intval_h = intval_h;
        ir_pulsecounter = pulsecounter;
        m_f_error = true;
}


void IR::loop(){ // transform raw data from IR to ir_result
    static uint16_t number = 0;
    static uint8_t idx = 0;

    if(ir_cmd_a != -01){
        if(ir_cmd_a + ir_cmd_b != 0xFF){
            return;
        }
        if(ir_code) ir_code(ir_adr_a, ir_cmd_a);
        // log_i("ir_adr_a %i  ir_adr_b %i ir_cmd_a %i  ir_cmd_b %i", ir_adr_a, ir_adr_b, ir_cmd_a, ir_cmd_b);
        if(ir_adr_a != ir_addressCode){ir_cmd_a = -01; return;}
        m_t0 = millis();
        bool found = false;
        for(uint8_t i = 0; i < 20; i++){
            if(ir_cmd_a == m_ir_buttons[i]){
                found = true;
                if(i <= 9){
                    if(idx > 2) break;
                    uint8_t digit = i;
                    number *= 10;
                    number += digit;
                    if(ir_number) ir_number(number);
                    idx++;
                }
                else{ // is not a number
                    if(ir_key) ir_key(i);
                    // log_i("ir m_key 0x%02x", i);
                    idx = 0;
                    m_key = i;
                }
                break;
            }
        }
        if(!found) log_w("No function has been assigned to the code 0x%02x", ir_cmd_a);
        ir_cmd_a = -01;
    }
    if(idx && (m_t0 + 2000 < millis())){
        idx = 0;
        if(ir_res) ir_res(number);
        number = 0;

    }
    if(m_key >= 0 && (m_t0 + 2000 < millis())){
        if(ir_rc > 14){
            if(ir_long_key) ir_long_key(m_key);
        }
        m_key = -1;
    }
    if(m_f_error){
        log_d("something went wrong, intval_l %d, intval_h %d, pulsecounter %d", ir_intval_l, ir_intval_h, ir_pulsecounter);
        m_f_error = false;
    }
}

//**************************************************************************************************
//                                          I S R _ I R                                            *
//**************************************************************************************************
// Interrupts received from VS1838B on every change of the signal.                                 *
// Intervals are 640 or 1640 microseconds for data.  syncpulses are 3400 micros or longer.         *
// Input is complete after 65 level changes.                                                       *
// Only the last 32 level changes are significant.                                                 *
//**************************************************************************************************
void IRAM_ATTR isr_IR(){

    extern IR ir;

    uint16_t        userCode = 0;                       // The first 4 bytes of IR code
    uint16_t        dataCode = 0;                       // The last 4 bytes of IR code
    int32_t         t1=0, intval_h=0, intval_l = 0;     // Current time and interval since last change

    static uint8_t  pulsecounter=0;                     // Counts the pulse
    static uint32_t t0 = 0;                             // To get the interval
    static uint32_t ir_value=0;                         // IR code
    static uint64_t bit = 0x00000001;

    static boolean f_AGC = 0;                          // AGC pulse 9000µs (negative)
    static boolean f_LP = 0;                           // LP pulse 4500µs (positive)
    static boolean f_BURST  = 0;                       // BURST pulse 562.5µs (negative)
    static boolean f_P = 0;                            // SPACE pulse 2250µs (positive), in repeat code
    static boolean f_RC = false;                       // repeat code sequence received
    static uint8_t RC_cnt = 0;                         // repeat code counter

    t1 = micros();                                      // Get current time
    if(!digitalRead(g_ir_pin)) intval_h = t1 - t0;      // Compute interval, only high
    else                       intval_l = t1 - t0;      // Compute interval, only low
    t0 = t1;                                            // Save for next compare

    if(intval_l >= 8000 && intval_l < 10000){           // 9000µs AGC
        f_AGC = true;
        f_LP = false;
        return;
    }

    if(f_AGC && !f_LP){
        if((intval_h >= 3500)&&(intval_h <= 5500)) {    // begin sequence of code?
            f_LP = true;
            f_AGC = false;
            pulsecounter = 0;                           // Reset counter
            ir_value = 0;
            bit = 0x00000001;
            RC_cnt = 0;
            return;
        }
        // else fall through
    }

    if(f_LP){
        if((intval_h > 400) && (intval_h < 750)){  // Logical '0' – a 562.5µs pulse burst followed by a 562.5µs space
            if(f_BURST){
                f_BURST = false;
                bit <<= 1;
                pulsecounter++;
                return;
            }
        }

        if((intval_h > 1500) && (intval_h < 1700)){ // Logical '1' – a 562.5µs pulse burst followed by a 1.6875ms space
            if(f_BURST){
                f_BURST = false;
                ir_value += bit;
                bit <<= 1;
                pulsecounter++;
            }
            return;
        }

        if((intval_l > 400) && (intval_l < 750)){
            f_BURST = true;
            if(pulsecounter < 32) return;
            // last p_BURST
            userCode =  ir_value & 0x0000FFFF;          // aka address
            dataCode = (ir_value & 0xFFFF0000) >> 16;   // aka command
            uint8_t a, b, c, d;
            a = (userCode & 0x00FF);      // Extended NEC protocol: Address high
            b = (userCode & 0xFF00) >> 8; // Extended NEC protocol: Address low
            d = (dataCode & 0xFF00) >> 8;
            c = (dataCode & 0x00FF);
            ir.setIRresult(a, b, c, d);

            pulsecounter++;
            f_LP = false;
            return;
        }
        // else fall through
    }


    if(f_AGC){  // repeat code
        if((intval_h > 1700) && (intval_h < 2800)){ // repeat code 2250µs
            f_P = true;
            return;
        }
       if(f_P){
            if((intval_l > 400) && (intval_l < 750)){
                f_AGC = false;
                f_RC = true;
            }
       }
    }
    if(f_RC){
        f_RC = false;
        RC_cnt++;
        ir.rcCounter(RC_cnt);
        return;
    }

    if(intval_l) ir.error( intval_l, intval_h, pulsecounter);
}
//------------------------------------------------------------------------------------------------------------------
