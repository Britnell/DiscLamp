#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

#include "orbit.h"

Orbit blue;
Orbit red;

uint8_t hues_initialised = 0;

void init_hues(){
    if(hues_initialised) return;
    hues_initialised = 1;
    
    blue.x = 2;
    blue.y = 2;
    red.x = -2;
    red.y = -2;
    
    blue.rad = 7;
    red.rad = 3.9;

    
    blue.vx = 0;
    blue.vy = 0;
    red.vx = 0;
    red.vy = 0;
    
    float ang = random_angle();
    blue.ax = cos(ang);
    blue.ay = sin(ang);
    ang = random_angle();
    red.ax = cos(ang);
    red.ay = sin(ang);
    
}

float get_dist(float xa, float ya, float xb, float yb){
    return abs( (xa-xb)*(xa-xb) + (ya-yb)*(ya-yb) );
}

void step_hues(){
    step_orbit(&blue);
    step_orbit(&red);
}

void draw_hues(){
    float glow = 2.2;
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
        else if(dist < red.rad + glow)
            c_r = linear( dist, red.rad,red.rad+glow, 255,0 );
        
        leds[l] = CRGB(c_r,0,c_b);
    }
    FastLED.show();
}

void hues(){

    init_hues();
    
    step_hues();
    draw_hues();

    delay(20);
}