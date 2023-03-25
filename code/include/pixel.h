#ifndef PIXEL_CLASS
#define PIXEL_CLASS

#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS  151
#define DATA_PIN   D4
#define CLOCK_PIN 0
#define N_ROWS 15

typedef struct {
  uint8_t l;    // actual led-string index
  uint8_t row;     // index row [vertical] 
  uint8_t row_i;      // array index in row
  uint8_t row_leds;   // leds in row
  float x;
  float y;
} LED_STRUCT;

CRGB leds[NUM_LEDS];
LED_STRUCT pixel[NUM_LEDS];

int leds_per_row[] = { 4, 7,10, 11, 12,13, 12,13, 12,13, 12,11, 10, 7, 4 };
const float DY = 0.866;


void init_pixel(){
    
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    
  int led_index = 0;
  
  for(uint8_t row=0; row<N_ROWS; row++)
  {
    int rowleds = leds_per_row[row];
    float x_offset;
    if(rowleds%2==0)
      x_offset = - (rowleds/2) +0.5 ;
    else
      x_offset = - (rowleds/2) ;
      
    
    for(uint8_t r=0;r<rowleds;r++){
      
      // struct
      LED_STRUCT led;
      led.l = led_index;
      
      led.row = row;
      led.row_leds = rowleds;
      if(row%2==0)
        led.row_i = r;
      else
        led.row_i = rowleds-1-r;
      
      if(row%2==0)
        led.x = x_offset + r ;
      else
        led.x = x_offset + (rowleds-1) - r ;

      led.y = ( N_ROWS - 8 - row ) * DY;
      
      // insert
      pixel[led_index] = led;
      led_index++;
      // Eo for led in row
    }
    
    // Eo for-rows
  }
  
  // Eo init
}



void demo(){

  for(int x=0; x<NUM_LEDS;x++){
    for(int i=0; i<NUM_LEDS;i++){
      if(x==i)
        leds[i] = CRGB::Red;
      else 
        leds[i] = CRGB::Black;

    }
    FastLED.show();
    delay(1);
  }
}

#endif