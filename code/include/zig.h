#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

#define zig_freq 6.0
#define zig_ang  1

void zig(){

  int val = 0;
  float r = 0.3;
  
    
  for(int l=0;l<NUM_LEDS;l++){
    float p;

    if(pixel[l].x>=0)
        p = (pixel[l].x + pixel[l].y *zig_ang +f ) / zig_freq ;
    else 
        p = (-pixel[l].x + pixel[l].y *zig_ang +f ) / zig_freq ;

    p = f_cap(p);

    val = 0;
    if(p>1-r)
        val = int( linear(p, 1-r,1, 0,255 )  );
    else if(p<r)
        val = int( linear(p, 0,r, 255,0)  );

    leds[l].setHSV( hue ,255,  val );
  }
  FastLED.show();  

  f -= 0.01;
  delay(10);

}