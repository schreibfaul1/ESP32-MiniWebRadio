/*
 * IR.h
 *
 *  Created on: 11.08.2017
 *      Author: Wolle
 *  Updated on: 05.01.2025
 */

#ifndef IR_H_
#define IR_H_

#include "Arduino.h"
#include <vector>
extern __attribute__((weak)) void ir_res(uint32_t res);
extern __attribute__((weak)) void ir_number(uint16_t);
extern __attribute__((weak)) void ir_short_key(int8_t);
extern __attribute__((weak)) void ir_long_key(int8_t);
extern __attribute__((weak)) void ir_code(uint8_t, uint8_t);
extern __attribute__((weak)) void ir_released(int8_t);

// prototypes
void isr_IR();

class IR {

    private:
        uint32_t m_t0;
        uint32_t m_t1;
        uint32_t m_ir_num = 0;
        int8_t   m_ir_pin;
        uint8_t  m_ir_resultstr[10];
        int8_t   m_short_key = -1;
        int8_t   m_released_key = -1;
        int8_t   m_long_key = -1;
        boolean  m_f_error = false;

    protected:
        int16_t m_ir_buttons[50];
    public:
        IR(int8_t IR_Pin);
        ~IR();
        void begin();
        void set_irButtons(uint8_t btnNr,  uint8_t cmd);
        int16_t* get_irButtons();
        void set_irAddress(uint8_t addr);
        int16_t get_irAddress(); // 0 ... 255 and -1 if not set
        void setIRresult(uint8_t userCode_a, uint8_t userCode_b, uint8_t dataCode_a, uint8_t dataCode_b);
        void rcCounter(uint8_t rc);
        void loop();
        void error(uint32_t intval_l, uint32_t intval_h, uint8_t pulsecounter);
};

#endif /* IR_H_ */
