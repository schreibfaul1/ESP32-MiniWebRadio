/*
 * IR.cpp
 *
 *  Created on: 11.08.2017
 *      Author: Wolle
 *  Updated on: 04.07.2022
 */
#include "IR.h"

// global var
DRAM_ATTR uint8_t ir_resp; // set from isr

IR::IR(uint8_t IR_PIN){
    ir_pin=IR_PIN;
    ir_result=0;
    ir_resp = 0xFF;
    t0=0;

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
    if(ir_pin >= 0){
        pinMode(ir_pin, INPUT_PULLUP);
        attachInterrupt(ir_pin, isr_IR, CHANGE); // Interrupts will be handle by isr_IR
    }
}
void IR::defineButtons(irBtn_t* b){
    uint8_t b_len = sizeof(b);
    for (int i = 0; i < b_len; i++){
        log_i("%d %c", b[i].val, b[i].ch);
    }
}

void IRAM_ATTR IR::setIRresult(uint8_t result){
    ir_resp = result;
}

void IR::loop(){ // transform raw data from IR to ir_result
    static uint16_t number = 0;
    static uint8_t idx = 0;

    if(ir_resp != 0xFF){
        t0 = millis();
        for(uint i = 0; i < 256; i++){
            if(ir_resp == ir_buttons[i].val){
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
        ir_resp = 0xFF;
    }
    if(idx && (t0 + 2000 < millis())){
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

    uint16_t        address=0;                  // The first 4 bytes of IR code
    uint16_t        command=0;                  // The last 4 bytes of IR code
    int32_t         t1=0, intval=0;             // Current time and interval since last change

    static uint8_t  levelcounter=0;             // Counts the level changes
    static uint8_t  pulsecounter=0;             // Counts the pulse
    static uint32_t t0=0;                       // To get the interval
    static uint32_t ir_value=0;                 // IR code
    static boolean  ir_begin=false;             // set if HIGH/LOW change
    static uint64_t bit = 0x00000001;

    t1=micros();                                // Get current time
    intval=t1 - t0;                             // Compute interval
    t0=t1;                                      // Save for next compare

    if((intval >= 3500)&&(intval <= 5500)) {    // begin sequence of code?
        pulsecounter=0;                         // Reset counter
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
        address= ir_value & 0xFFFF;
        if(address==0x00FF){
            command=(ir_value & 0xFFFF0000) >> 16;
            uint8_t a, b;
            a = (command & 0xFF00) >> 8;
            b = (command & 0x00FF);
            if(a + b == 0xFF){
                ir.setIRresult(a);
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
