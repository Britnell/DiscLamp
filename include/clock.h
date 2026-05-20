#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"
#include "serial.h"

uint8_t h = 3;
uint8_t m = 3;
uint8_t s = 0;

void tick(){
    s++;
    if(s<=60) return;
    s = 1;    m++;
    if(m<=60) return;
    m = 1;    h++;
    if(h<=12) return;
    h = 1;
}

void clock_1(){
  uint8_t val, l, b;
  for(l=0; l<NUM_LEDS; l++){
    leds[l] = CRGB(0,0,0);
  }
  for(b=0; b<6; b++){
    val = (s & (0x01 << b )) ? 255 : 0;
    leds[75-b].setHSV(hue,250,val);
    leds[75+b].setHSV(hue,250,val);

    val = (m & (0x01 << b )) ? 255 : 0;
    leds[100-b].setHSV(hue,250,val);
    leds[100+b].setHSV(hue,250,val);
    leds[50-b].setHSV(hue,250,val);
    leds[50+b].setHSV(hue,250,val);

    if(b<4) {
    val = (h & (0x01 << b )) ? 255 : 0;
    leds[26-b].setHSV(hue,250,val);
    leds[26+b].setHSV(hue,250,val);
    leds[124-b].setHSV(hue,250,val);
    leds[124+b].setHSV(hue,250,val);
    }
  }
  FastLED.show();  
}



void clock_loop(){
    if(millis()-timer >= 1000){
        timer = millis();
        tick();
        clock_1();
    }
    // delay(2);
}