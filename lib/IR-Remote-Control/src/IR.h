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
extern __attribute__((weak)) void ir_number(uint16_t);
extern __attribute__((weak)) void ir_key(uint8_t);
extern __attribute__((weak)) void ir_long_key(int8_t);
extern __attribute__((weak)) void ir_code(uint8_t, uint8_t);

// prototypes
void IRAM_ATTR isr_IR();

class IR {

    private:
        uint32_t m_t0;
        uint32_t m_ir_num = 0;
        int8_t   m_ir_pin;
        uint8_t  m_ir_resultstr[10];
        int8_t   m_key = -1;

    protected:
        uint8_t m_ir_buttons[20];
    public:
        IR(uint8_t IR_PIN);
        ~IR();
        void begin();
        void set_irButtons(uint8_t btnNr,  uint8_t cmd);
        uint8_t* get_irButtons();
        void set_irAddress(uint8_t addr);
        uint8_t get_irAddress();
        void setIRresult(uint8_t userCode, uint8_t dataCode);
        void rcCounter(uint8_t rc);
        void loop();


};

#endif /* IR_H_ */
