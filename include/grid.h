#ifndef GRID_H
#define GRID_H

#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

// static grid: centre pixel on, then every 2nd pixel on each row

void grid(){

  for(int l=0;l<NUM_LEDS;l++){
    int gx = (int)round(pixel[l].x);
    int val;

    if(!invert){
      val = 0;
      if(pixel[l].hy % 2 == 0  &&  gx % 2 == 0)   val = 255;
    }
    else {
      val = 255;
      if(pixel[l].hy % 2 == 0  &&  gx % 2 == 0)   val = 0;
    }

    leds[l].setHSV( grad_hue(pixel[l].x, pixel[l].y) ,250, val );
  }
  FastLED.show();

  delay(100);
}

#endif
