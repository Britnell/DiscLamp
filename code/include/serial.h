#ifndef serial_comms
#define serial_comms

#include <Arduino.h>
#include "pattern.h"
#include "lib.h"
#include "pixel.h"

char pbuffer[100];
String serial_in = "";

void cmd_line(String str){
    char cmd = str.charAt(0);
    str.remove(0,1);
    if(cmd=='m'){
        mode = str;
        p(" mode = ");
        pl(str);
        return;
    }
    if(cmd=='b'){
        bright = str.toInt();
        FastLED.setBrightness(bright);
        p(" bright = ");
        pl(bright);
        return;
    }    
    if(cmd=='h'){
        hue = str.toInt();
        p(" hue = ");
        pl(hue);
        return;
    }
    pl(str);
}

void read_serial(){
    while(Serial.available()>0){
        char c = Serial.read();
        if(c=='\n'){
            cmd_line(serial_in);
            serial_in = "";
        }
        else serial_in += c;
    }
}


#endif