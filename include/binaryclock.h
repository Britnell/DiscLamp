#ifndef BINARYCLOCK_H
#define BINARYCLOCK_H

#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"
#include "serial.h"
#include "clock.h"

void binaryclock_1(){
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

void binaryclock_loop(){
    if(millis()-timer >= 1000){
        timer = millis();
        tick();
        binaryclock_1();
    }
}

#endif
