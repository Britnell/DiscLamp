#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"


// * RAIN

#define RAIN_MAX_COUNT  18
#define RAIN_MIN_COUNT   5
#define RAIN_MIN_RAD   0.6f
#define RAIN_MAX_RAD   4.2f

float rain_speed = 0.04;

uint8_t num_rain = 10;
float   rain_rad = 1.0f;
uint8_t rain_initialised = 0;

typedef struct {
  float x;
  float y;
} RAIN_PARTICLE;

RAIN_PARTICLE particles[RAIN_MAX_COUNT];

void rain_reshuffle(){
  // scale 0..100: 0 = few big particles, 100 = many small particles
  uint8_t scale = 50; //random(0, 101);
  num_rain = (uint8_t)linear(scale, 0, 100, RAIN_MIN_COUNT, RAIN_MAX_COUNT);
  rain_rad = linear(scale, 0, 100, RAIN_MAX_RAD, RAIN_MIN_RAD);

  for(int r=0; r<num_rain; r++){
    particles[r].x = linear(random(0,255), 0, 255, -6, 6);
    particles[r].y = linear(r, 0, num_rain+1, -6, 6);
  }
  rain_initialised = 1;
}

void rain(){
  if(!rain_initialised) rain_reshuffle();
  int val;
  float x, d, dist;

  // * pixels
  for(int l=0;l<NUM_LEDS;l++){
    dist = 10;
    for(int r=0;r<num_rain;r++){
      x = pixel[l].x-particles[r].x;
      d = x*x;
      x = pixel[l].y -particles[r].y;
      d += x*x;
      if(d<dist)  dist = d;
    }

    val = 0;
    if(dist<rain_rad)
      val = int(expon( (rain_rad-dist), 0,rain_rad, 0,255) );

    leds[l].setHSV( grad_hue(pixel[l].x, pixel[l].y) ,250,  val );
  }
  FastLED.show();

  // * step (falling down: -y)
  for(int r=0;r<num_rain;r++){
    particles[r].y -= rain_speed;

    if(particles[r].y < -6) {
      float min_sep = rain_rad * 1.5f;
      float nx = 0, ny = 6;
      for(int tries=0; tries<20; tries++){
        nx = linear(random(0,255),0,255,-6,6);
        bool ok = true;
        for(int o=0; o<num_rain; o++){
          if(o==r) continue;
          float dx = nx - particles[o].x;
          float dy = ny - particles[o].y;
          if(dx*dx + dy*dy < min_sep*min_sep){ ok = false; break; }
        }
        if(ok) break;
      }
      particles[r].x = nx;
      particles[r].y = ny;
    }
  }
  delay(20);
}
