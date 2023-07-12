/*
 * IR.h
 *
 *  Created on: 11.08.2017
 *      Author: Wolle
 */

#ifndef IR_H_
#define IR_H_

#include "Arduino.h"
#include <vector>
extern __attribute__((weak)) void ir_res(uint32_t res);
extern __attribute__((weak)) void ir_number(const char*);
extern __attribute__((weak)) void ir_key(const char*);

// prototypes
void IRAM_ATTR isr_IR();

struct ir_btn{
    uint8_t val;
    char    ch;
};
typedef ir_btn irBtn_t;

class IR {

    private:
        boolean  f_entry=false;  // entryflag
        boolean  f_send=false;   // entryflag
        uint32_t t0;
        uint32_t ir_num=0;
        int8_t   ir_pin;
        uint8_t  ir_result;
        uint8_t  idx=0;
        char     ir_resultstr[10];
        uint16_t downcount=0;
        int8_t   tmp_resp =(-1);

    protected:
        irBtn_t ir_buttons[20];
    public:
        IR(uint8_t IR_PIN);
        ~IR();
        void begin();
        void defineButtons(irBtn_t* b);
        void setIRresult(uint8_t result);
        void loop();


};

#endif /* IR_H_ */
