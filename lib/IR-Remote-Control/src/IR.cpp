/*
 * IR.cpp
 *
 *  Created on: 11.08.2017
 *      Author: Wolle
 *  Updated on: 13.07.2023
 */
#include "IR.h"

// global var
DRAM_ATTR int16_t ir_dataCode; // set from isr
DRAM_ATTR uint8_t ir_userCode; // set from isr
DRAM_ATTR uint8_t g_ir_pin;

IR::IR(uint8_t IR_PIN){
    m_ir_pin = IR_PIN;
    ir_dataCode = -01;
    m_t0 = 0;
    ir_userCode = 0x00;

    ir_buttons[ 0].val = 0x52; ir_buttons[ 0].ch = '0';
    ir_buttons[ 1].val = 0x16; ir_buttons[ 1].ch = '1';
    ir_buttons[ 2].val = 0x19; ir_buttons[ 2].ch = '2';
    ir_buttons[ 3].val = 0x0D; ir_buttons[ 3].ch = '3';
    ir_buttons[ 4].val = 0x0C; ir_buttons[ 4].ch = '4';
    ir_buttons[ 5].val = 0x18; ir_buttons[ 5].ch = '5';
    ir_buttons[ 6].val = 0x5E; ir_buttons[ 6].ch = '6';
    ir_buttons[ 7].val = 0x08; ir_buttons[ 7].ch = '7';
    ir_buttons[ 8].val = 0x1C; ir_buttons[ 8].ch = '8';
    ir_buttons[ 9].val = 0x5A; ir_buttons[ 9].ch = '9';
    ir_buttons[10].val = 0x40; ir_buttons[10].ch = 'o';  // OK
    ir_buttons[11].val = 0x46; ir_buttons[11].ch = 'u';  // UP
    ir_buttons[12].val = 0x15; ir_buttons[12].ch = 'd';  // DOWN
    ir_buttons[13].val = 0x43; ir_buttons[13].ch = 'r';  // RIGHT
    ir_buttons[14].val = 0x44; ir_buttons[14].ch = 'l';  // LEFT
    ir_buttons[15].val = 0x4A; ir_buttons[15].ch = '#';  // #
    ir_buttons[16].val = 0x42; ir_buttons[16].ch = '*';  // *
    ir_buttons[17].val = 0x00; ir_buttons[17].ch = '0';
    ir_buttons[18].val = 0x00; ir_buttons[18].ch = '0';
    ir_buttons[19].val = 0x00; ir_buttons[19].ch = '0';
}

IR::~IR(){

}

void IR::begin(){
    if(m_ir_pin >= 0){
        g_ir_pin = m_ir_pin;
        pinMode(m_ir_pin, INPUT_PULLUP);
        attachInterrupt(m_ir_pin, isr_IR, CHANGE); // Interrupts will be handle by isr_IR
    }
}
void IR::set_irButtons(irBtn_t* b){
    uint8_t b_len = 20;
    for (int i = 0; i < b_len; i++){
        // log_i("Button [%i] val=%d   ch=%c",i, b[i].val, b[i].ch);
        ir_buttons[i].val = b[i].val;
        ir_buttons[i].ch = b[i].ch;
    }
}

irBtn_t* IR::get_irButtons(){
    return ir_buttons;
}

void IR::set_irAddress(uint8_t addr){
    ir_userCode = addr;
}

uint8_t IR::get_irAddress(){
    return ir_userCode;
}


void IRAM_ATTR IR::setIRresult(uint8_t userCode, uint8_t dataCode){
    ir_dataCode = dataCode;
    ir_userCode = userCode;
}

void IR::loop(){ // transform raw data from IR to ir_result
    static uint16_t number = 0;
    static uint8_t idx = 0;

    if(ir_dataCode != -01){
        char adr[5];
        char cmd[5];
        sprintf(adr, "0x%02X", ir_userCode);
        sprintf(cmd, "0x%02X", ir_dataCode);
        if(ir_code) ir_code(adr, cmd, ir_userCode, ir_dataCode);
        if(ir_userCode != 0x00){ir_dataCode = -01; return;}
        m_t0 = millis();
        for(uint i = 0; i < 256; i++){
            if(ir_dataCode == ir_buttons[i].val){
                uint8_t ch = ir_buttons[i].ch;
                if(ch >= '0' && ch <= '9'){
                    if(idx > 2) break;
                    uint8_t digit = ch - 48;
                    number *= 10;
                    number += digit;
                    char buf[5]; itoa(number, buf, 10);
                    if(ir_number) ir_number(buf);
                    idx++;
                }
                else{ // is not a number
                    ir_resultstr[0] = (uint8_t)ch;
                    ir_resultstr[1] = '\0';
                    if(ir_key) ir_key(ir_resultstr);
                    idx = 0;
                }
                break;
            }
        }
        ir_dataCode = -01;
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
        log_i("ir_value %x", ir_value);
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
