#ifndef serial_comms
#define serial_comms

#include <Arduino.h>
#include "pattern.h"
#include "lib.h"
#include "pixel.h"

void snake_reshuffle();
void rain_randomize();
void rain_restart();
void init_autom();

extern uint8_t h, m, s;

char pbuffer[100];
String serial_in = "";

void cmd_line(String str){
    char cmd = str.charAt(0);
    str.remove(0,1);
    if(cmd=='p'){
        param = (uint8_t)str.toInt();
        rain_restart();
        return;
    }
    if(cmd=='m'){
        mode = str;
        if(str.equals("snake")) snake_reshuffle();
        if(str.equals("rain"))  rain_randomize();
        if(str.equals("autom")) init_autom();
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
        uint8_t diff = hue_diff(hue);
        hue_a = (hue + diff) % 256;
        hue_b = (hue + 256 - diff) % 256;
        p(" hue = ");
        pl(hue);
        return;
    }
    if(cmd=='i'){
        invert = str.toInt();
        p(" invert = ");
        pl(invert);
        return;
    }
    if(cmd=='t'){
        int th, tm, ts;
        if(sscanf(str.c_str(), "%d:%d:%d", &th, &tm, &ts) == 3){
            h = th; m = tm; s = ts;
        }
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