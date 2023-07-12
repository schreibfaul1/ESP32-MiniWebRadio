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
extern __attribute__((weak)) void ir_code(const char*, const char*, uint8_t, uint8_t);

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
        uint32_t m_t0;
        uint32_t m_ir_num = 0;
        int8_t   m_ir_pin;
        char     ir_resultstr[10];
        int8_t   tmp_resp =(-1);

    protected:
        irBtn_t ir_buttons[20];
    public:
        IR(uint8_t IR_PIN);
        ~IR();
        void begin();
        void set_irButtons(irBtn_t* b);
        irBtn_t* get_irButtons();
        void set_irAddress(uint8_t addr);
        uint8_t get_irAddress();
        void setIRresult(uint8_t userCode, uint8_t dataCode);
        void loop();


};

#endif /* IR_H_ */
