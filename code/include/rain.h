#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"


// * RAIN

uint8_t rain_initialised = 0;
#define NUM_RAIN  10

float rain_rad = 1;
float rain_speed = 0.04;

typedef struct {
  float x;
  float y;
} RAIN_PARTICLE;


RAIN_PARTICLE particles[NUM_RAIN];

void rain(){
  RAIN_PARTICLE p;

  if(!rain_initialised){
    for(int r=0;r<NUM_RAIN;r++){
      p.x = linear(random(0,255),0,255,-6,6);
      p.y = linear(r,0,NUM_RAIN+1,-6,6);
      particles[r] = p;
    }
    rain_initialised = 1;
  }

  int val;
  float x, d, dist;
  
  // * pixels
  for(int l=0;l<NUM_LEDS;l++){
    dist = 10;
    for(int r=0;r<NUM_RAIN;r++){
      x = pixel[l].x-particles[r].x;
      d = x*x;
      x = pixel[l].y -particles[r].y;
      d += x*x;
      if(d<dist)  dist = d;
    }

    val = 0;
    if(dist<rain_rad)
      val = int(expon( (rain_rad-dist), 0,rain_rad, 0,255) );

    leds[l].setHSV( hue ,250,  val );
  }
  FastLED.show();  

  // * step
  for(int r=0;r<NUM_RAIN;r++){
    particles[r].y += rain_speed;

    if(particles[r].y>6) {
      particles[r].x = linear(random(0,255),0,255,-6,6);
      particles[r].y = -6;
      // particles[r].y = linear(random(0,255),0,255,-6,-8);
    }
  }
  delay(20);
}
