#ifndef RING_H
#define RING_H

#include "pixel.h"
#include "pattern.h"
#include <math.h>

bool in_ring(int l, float radius, float width) {
  float d = sqrtf(pixel[l].x * pixel[l].x + pixel[l].y * pixel[l].y);
  return fabsf(d - radius) <= width;
}

// Colours ring LEDs using the gradient hue
void draw_radius(float radius, float width) {
  for (int l = 0; l < NUM_LEDS; l++) {
    if (in_ring(l, radius, width))
      leds[l].setHSV(grad_hue(pixel[l].x, pixel[l].y), 250, 255);
  }
}

// Colours ring LEDs with a fixed colour
void draw_ring_col(float radius, float width, CRGB col) {
  for (int l = 0; l < NUM_LEDS; l++) {
    if (in_ring(l, radius, width))
      leds[l] = col;
  }
}

void draw_ring(uint8_t r){
    if(r==0){
      int l = 75;
      leds[l].setHSV(grad_hue(pixel[l].x, pixel[l].y), 250, 255);
    } 
    else if(r==1){
        draw_radius(1.1f, 0.4f);
    }
    else if(r==2) {
        draw_radius(1.6f, 0.4f);
    }
    else if(r==3) {
        draw_radius(2.6f, 0.5f);
    }
    else if(r==4) {
        draw_radius(3.6f, 0.5f);
    }
    else if(r==5){
        draw_radius(4.6f, 0.5f);
    }
    else if(r==6){
        draw_radius(5.55f, 0.45f);
    }
}

#endif
