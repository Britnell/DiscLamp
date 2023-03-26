#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

#include "orbit.h"

Orbit blue;
Orbit red;

uint8_t hues_initialised = 0;

void init_hues(){
    blue.x = 0;
    blue.y = 3;
    blue.rad = 2.8;
    blue.vx = 0;
    blue.vy = 0;
    
    red.x = 0;
    red.y = -3;
    red.rad = 2.8;
    red.vx = 0;
    red.vy = 0;
    
}

float get_dist(float xa, float ya, float xb, float yb){
    return abs( (xa-xb)*(xa-xb) + (ya-yb)*(ya-yb) );
}

void hues(){

    if(!hues_initialised){
        init_hues();
        hues_initialised = 1;
    }

    float glow = 0.9;
    float dist;

    for(int l=0;l<NUM_LEDS;l++){
        int c_b = 0;
        int c_r = 0;
        
        dist = get_dist( pixel[l].x, pixel[l].y, blue.x,blue.y );
        if(dist<blue.rad)
            c_b = 255;
        else if(dist < blue.rad + glow)
            c_b = linear( dist, blue.rad,blue.rad+glow, 255,0 );

        dist = get_dist( pixel[l].x, pixel[l].y, red.x,red.y );
        if(dist<red.rad)
            c_r = 255;
        else if(dist < blue.rad + glow)
            c_r = linear( dist, blue.rad,blue.rad+glow, 255,0 );
        
        leds[l] = CRGB(c_r,0,c_b);
    }
    FastLED.show();

    delay(100);
}