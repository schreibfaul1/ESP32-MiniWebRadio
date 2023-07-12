# ESP32-IR-Remote-Control
C++ class for the infrared remote control and reciever module 

![Arduino IR kit](https://github.com/schreibfaul1/ESP32-IR-Remote-Control/blob/master/images/Infrarot%20IR%20Empf%C3%A4nger%20Modul%20Wireless%20Remote%20Control%20Kit%20f%C3%BCr%20Arduino.jpg)

### Examplecode:

```` c++
#include "Arduino.h"
#include "IR.h"

#define IR_PIN  34
IR ir(IR_PIN);  // do not change the objectname, it must be "ir"

//--------------------------------------------------------------
void setup()
{
    ir.begin();  // Init InfraredDecoder
    Serial.begin(115200);
}
void loop()
{
    ir.loop();
    // do something else
    // ...
}
//--------------------------------------------------------------
//   events called from IR Library
//--------------------------------------------------------------
void ir_res(uint32_t res){
    Serial.print("ir_res: ");
    Serial.println(res);
}

void ir_number(const char* num){
    Serial.print("ir_number: ");
    Serial.println(num);
}

void ir_key(const char* key){
    Serial.print("ir_key: ");
    Serial.println(key);
}
````
### console output (pressed # 123):

![Serial Console Output](https://github.com/schreibfaul1/ESP32-IR-Remote-Control/blob/master/images/IR_ConsoleOutput.jpg)

### recieved codewods for this RC:

![RC Code](https://github.com/schreibfaul1/ESP32-IR-Remote-Control/blob/master/images/RemoteControl.jpg)

### VS1838 pins;

![VS1838B Pins](https://github.com/schreibfaul1/ESP32-IR-Remote-Control/blob/master/images/VS1838B%20Pins.jpg)
