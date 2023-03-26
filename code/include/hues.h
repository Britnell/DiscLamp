#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

#include "orbit.h"

Orbit blue;
Orbit red;
Orbit green;
float orbit_glow;
uint8_t hues_initialised = 0;


void init_hues(){
    if(hues_initialised) return;
    hues_initialised = 1;
    
    blue.rad = 10;
    red.rad = 7;
    green.rad = 0.4;
    orbit_glow = 2.6;

    blue.x = 2;    blue.y = 2;
    red.x = -2;    red.y = -2;
    green.x = -4;    green.y = 0;

    blue.vx = 0;    blue.vy = 0;
    red.vx = 0;     red.vy = 0;
    green.vx = 0;     green.vy = 0;
    
    float ang = random_angle();
    blue.ax = cos(ang);    blue.ay = sin(ang);
    ang = random_angle();
    red.ax = cos(ang);     red.ay = sin(ang);
    ang = random_angle();
    green.ax = cos(ang);     green.ay = sin(ang);
}

float get_dist(float xa, float ya, float xb, float yb){
    return abs( (xa-xb)*(xa-xb) + (ya-yb)*(ya-yb) );
}


void draw_hues(){
    float dist;
    for(int l=0;l<NUM_LEDS;l++){
        CRGB col = CRGB(0,0,0);

        int val;
        val = 0;
        dist = get_dist( pixel[l].x, pixel[l].y, blue.x,blue.y );
        if(dist<blue.rad)
            val = 255;
        else if(dist < blue.rad + orbit_glow)
            val = linear( dist, blue.rad,blue.rad + orbit_glow, 255,0 );
        col += CRGB(0,0,val);

        val = 0;
        dist = get_dist( pixel[l].x, pixel[l].y, red.x,red.y );
        if(dist<red.rad)
            val = 255;
        else if(dist < red.rad + orbit_glow)
            val = linear( dist, red.rad,red.rad + orbit_glow, 255,0 );
        col += CRGB(val,0,0);
        
        val = 0;
        dist = get_dist( pixel[l].x, pixel[l].y, green.x,green.y );
        if(dist<green.rad)
            val = 80;
        else if(dist < green.rad + orbit_glow)
            val = linear( dist, green.rad,green.rad + orbit_glow, 120,0 );
        col += CRGB(0,val*0.8,val);
        
        leds[l] = col;
    }
    FastLED.show();
}

void hues(){

    init_hues();
    
    step_orbit(&blue);
    step_orbit(&red);
    step_orbit(&green);

    draw_hues();

    delay(10);
}