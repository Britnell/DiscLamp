#ifndef DEPRECATED_H
#define DEPRECATED_H

// Old patterns kept for reference. Not wired into the dispatcher.
// Include this header and add the mode dispatch entry to revive any of these.

#include "pixel.h"
#include "lib.h"
#include "pattern.h"

void hatch(){
  for(int l=0;l<NUM_LEDS;l++){

    int y = round(pixel[l].y / DY ) + 10*3;
    int x = round( pixel[l].x );
    int h = round( pixel[l].x + 0.5) + 10*3;

    int val = 0;
    if(y%6==0){
      if(x%3==0)    val = 255;
    }
    if(y%6==1 || y%6==5){
      if(h%3==0)    val = 255;
      if(h%3==1)    val = 255;
    }
    else if(y%6==2 || y%6==4){
      if(x%3!=0)  val = 255;
    }
    else if(y%6==3){
      if(h%3==2)  val = 255;
    }

    if(invert){
      if(val==0) val = 255;
      else       val = 0;
    }

    leds[l].setHSV( grad_hue(pixel[l].x, pixel[l].y) ,250, val );
  }
  FastLED.show();

  delay(100);
}


void hatch2(){

  for(int l=0;l<NUM_LEDS;l++){

    int y = round(pixel[l].y / DY ) + 10*3  +6;  // +2 +6
    int x = round( pixel[l].x ) + 10*3;
    int h = round( pixel[l].x + 0.5) + 10*3;

    int val = 0;
    if(y%8==0){
      if(x%4==0)                val = 255;
    }
    else if(y%8==1 || y%8==7){
      if(h%4==0 || h%4==1)      val = 255;
    }
    else if(y%8==2|| y%8==6){
      if(x%4==1 || x%4==3)      val = 255;
    }
    else if(y%8==3|| y%8==5){
      if(h%4==2 || h%4==3)      val = 255;
    }
    else if(y%8==4){
      if(x%4==2)                val = 255;
    }

    if(invert){
      if(val==0) val = 255;
      else       val = 0;
    }
    leds[l].setHSV( grad_hue(pixel[l].x, pixel[l].y) ,250, val );
  }
  FastLED.show();

  delay(100);
}


void arrow(){
  int Q = 3;
  int val,x,row;

  for(int l=0;l<NUM_LEDS;l++){
    val = 0;
    row = abs( int(pixel[l].y/DY) );

    if(row%2==0){
      x = abs( int(pixel[l].x) );
      if((x-row/2+10*Q)%Q==0)    val = 255;
    }
    else{
      x = abs(int(pixel[l].x));
      if((x-row/2+10*Q)%Q==0)    val = 255;
    }

    if(invert){
      if(val==0) val = 255;  else  val = 0;
    }
    leds[l].setHSV( grad_hue(pixel[l].x, pixel[l].y) ,255,  val );
  }
  FastLED.show();

  delay(100);
}


static uint8_t triforce[45] = {
  33,34,35,36,37,38,39,40,41,42,
  46, 47, 48, 49,   51,52,53,54,
  59,60,61,         64,65,66,
  72,73,            77,78,
  85,               90,
  98, 99, 100, 101, 102,
  111, 112, 113, 114,
  123,124,125,
  134,135,
  143
};

void trif(){
  int val;
  for(int l=0;l<NUM_LEDS;l++){
    val = 0;
    for(int t=0;t<45;t++){
      if(triforce[t]==l){
        val = 255;
        break;
      }
      if(triforce[t]>l)
        break;
    }
    leds[l].setHSV( grad_hue(pixel[l].x, pixel[l].y) ,255,  val );
  }
  FastLED.show();
  delay(100);
}


void line(){
  float w = 1.8;
  float r = 2.2;
  float m = 0.86;

  for(int l=0;l<NUM_LEDS;l++){
    float d = pixel[l].x - m * pixel[l].y;
    if(d<0) d = -d;
    int val;

    if(!invert){
      if(d<w)
        val = 255;
      else if(d<w+r)
        val = int( linear(d, w,w+r, 255,0) );
      else
        val = 0;
    }
    else {
      if(d<w)
        val = 0;
      else if(d<w+r)
        val = int( linear(d, w,w+r, 0,255) );
      else
        val = 255;
    }
    leds[l].setHSV( grad_hue(pixel[l].x, pixel[l].y),250, val );
  }
  FastLED.show();

  delay(100);
}

#endif
