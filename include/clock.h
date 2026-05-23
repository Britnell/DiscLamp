#ifndef CLOCK_H
#define CLOCK_H

#define CLOCK_DEBUG 0  // 1 = show mm:ss, 0 = show hh:mm

#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"
#include "serial.h"
#include "digit.h"

uint8_t h = 10;
uint8_t m = 23;
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

void draw_digit(uint8_t d, int8_t hx, int8_t hy, uint8_t col) {
  switch(d) {
    case 0: draw0(hx, hy, col); break;
    case 1: draw1(hx, hy, col); break;
    case 2: draw2(hx, hy, col); break;
    case 3: draw3(hx, hy, col); break;
    case 4: draw4(hx, hy, col); break;
    case 5: draw5(hx, hy, col); break;
    case 6: draw6(hx, hy, col); break;
    case 7: draw7(hx, hy, col); break;
    case 8: draw8(hx, hy, col); break;
    case 9: draw9(hx, hy, col); break;
  }
}

void clock_loop(){
    if(millis()-timer >= 1000){
        timer = millis();
        tick();

        for (int l = 0; l < NUM_LEDS; l++) leds[l] = CRGB::Black;

        uint8_t a = CLOCK_DEBUG ? m : h;
        uint8_t b = CLOCK_DEBUG ? s : m;

        draw_digit(a / 10, -7, 1,  hue);
        draw_digit(a % 10,  1, 1,  hue);
        draw_digit(b / 10, -7, -5, hue);
        draw_digit(b % 10,  1, -5, hue);

        FastLED.show();
    }
}

#endif
