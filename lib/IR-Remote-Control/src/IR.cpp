/*
 * IR.cpp
 *
 *  Created on: 11.08.2017
 *      Author: Wolle
 *  Updated on: 14.07.2023
 */
#include "IR.h"

// global var
DRAM_ATTR int16_t ir_cmd = -1; // set from isr
DRAM_ATTR int16_t ir_adr = 0;  // set from isr
DRAM_ATTR uint8_t ir_addressCode;
DRAM_ATTR uint8_t g_ir_pin;

IR::IR(uint8_t IR_PIN){
    m_ir_pin = IR_PIN;
    ir_adr = -1;
    m_t0 = 0;
    ir_cmd = -1;

    ir_buttons[ 0] = 0x00; //
    ir_buttons[ 1] = 0x00; //
    ir_buttons[ 2] = 0x00; //
    ir_buttons[ 3] = 0x00; //
    ir_buttons[ 4] = 0x00; //
    ir_buttons[ 5] = 0x00; //
    ir_buttons[ 6] = 0x00; //
    ir_buttons[ 7] = 0x00; //
    ir_buttons[ 8] = 0x00; //
    ir_buttons[ 9] = 0x00; //
    ir_buttons[10] = 0x00; //  mute
    ir_buttons[11] = 0x00; //  volume+
    ir_buttons[12] = 0x00; //  volume-
    ir_buttons[13] = 0x00; //  previous station
    ir_buttons[14] = 0x00; //  next station
    ir_buttons[15] = 0x00; //
    ir_buttons[16] = 0x00; //
    ir_buttons[17] = 0x00; //
    ir_buttons[18] = 0x00; //
    ir_buttons[19] = 0x00; //
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
        ir_buttons[btnNr] = cmd;
        // log_w("ButtonNumber: %i, Command: 0x%02x", btnNr, cmd);
}

uint8_t* IR::get_irButtons(){
    return ir_buttons;
}

void IR::set_irAddress(uint8_t addr){
    ir_addressCode = addr;
}

uint8_t IR::get_irAddress(){
    return ir_addressCode;
}


void IRAM_ATTR IR::setIRresult(uint8_t ir_userCode, uint8_t ir_dataCode){
    ir_cmd = ir_dataCode;
    ir_adr = ir_userCode;
}

void IR::loop(){ // transform raw data from IR to ir_result
    static uint16_t number = 0;
    static uint8_t idx = 0;

    if(ir_cmd != -01){
        if(ir_code) ir_code(ir_adr, ir_cmd);
        if(ir_adr != ir_addressCode){ir_cmd = -01; return;}
        m_t0 = millis();
        bool found = false;
        for(uint8_t i = 0; i < 20; i++){
            // log_i("ir_cmd %i ir_buttons[i] %i", ir_cmd, ir_buttons[i]);
            if(ir_cmd == ir_buttons[i]){
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
                    // log_i("ir key 0x%02x", i);
                    idx = 0;
                }
                break;
            }
        }
        if(!found) log_w("No function has been assigned to the code 0x%02x", ir_cmd);
        ir_cmd = -01;
    }
    if(idx && (m_t0 + 2000 < millis())){
        idx = 0;
        if(ir_res) ir_res(number);
        number = 0;
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
void IRAM_ATTR isr_IR()
{
    extern IR ir;

    uint16_t        userCode=0;                     // The first 4 bytes of IR code
    uint16_t        dataCode=0;                     // The last 4 bytes of IR code
    int32_t         t1=0, intval=0;                 // Current time and interval since last change

    static uint8_t  levelcounter=0;                 // Counts the level changes
    static uint8_t  pulsecounter=0;                 // Counts the pulse
    static uint32_t t0 = 0;                         // To get the interval
    static uint32_t ir_value=0;                     // IR code
    static boolean  ir_begin=false;                 // set if HIGH/LOW change
    static uint64_t bit = 0x00000001;

    t1 = micros();                                  // Get current time
    if(!digitalRead(g_ir_pin)) intval = t1 - t0;    // Compute interval, only high
    t0 = t1;                                        // Save for next compare
    if((intval >= 3500)&&(intval <= 5500)) {        // begin sequence of code?
        pulsecounter=0;                             // Reset counter
        ir_value=0;
        levelcounter=0;
        ir_begin=true;
        bit = 0x00000001;
        return;
    }

    if(ir_begin==false) return;
    levelcounter++;
    if(levelcounter%2==1)return;                // only falling edge can pass

    if(pulsecounter==32){

        ir_begin=false;
        userCode = ir_value & 0xFFFF;
        uint8_t a, b, c, d;
        a = (userCode & 0xFF00) >> 8;
        b = (userCode & 0x00FF);
        if(a + b == 0xFF){
            dataCode=(ir_value & 0xFFFF0000) >> 16;
            c = (dataCode & 0xFF00) >> 8;
            d = (dataCode & 0x00FF);
            if(c + d == 0xFF){
                ir.setIRresult(a, c);
            }
        }
    }
    if((intval > 400) && (intval < 750)){       // Short pulse?
        ir_value += bit;                        // Count number of received bits
        pulsecounter++;
        bit <<= 1;
    }
    else if((intval > 1400) && (intval < 1900)){     // Long pulse?
        pulsecounter++;
        bit <<= 1;
    }
    else{
        pulsecounter = 0;
        bit = 1;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
