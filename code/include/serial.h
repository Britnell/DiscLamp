#ifndef serial_comms
#define serial_comms

#include <Arduino.h>
#include "pattern.h"

#define p(x) Serial.print(x)
#define pl(x) Serial.println(x)

String ser_buf = "";

void cmd_line(String str){
    
    char cmd = str.charAt(0);
    str.remove(0,1);
    if(cmd=='m'){
        mode = str;
        Serial.print(" m= ");
        Serial.print(str);
    }
    else 
    Serial.print(str);
}

void read_serial(){
    while(Serial.available()>0){
        char c = Serial.read();
        if(c=='\n'){
            cmd_line(ser_buf);
            ser_buf = "";
        }
        else ser_buf += c;
    }
}


#endif