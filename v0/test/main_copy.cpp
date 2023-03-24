#include "secrets.h"
#include "func.h"

#include "connect.h"

#include "pixel.h"

#include "patterns.h"


// --------------------- - - - ---------------------- - - - ---------------------- - - - -

void setup() {

  init_pixel();
  
  leds[62].setRGB(0,0,20);
  leds[63].setRGB(0,0,20);
  leds[74].setRGB(0,0,20);
  leds[75].setRGB(0,0,250);
  leds[76].setRGB(0,0,20);
  FastLED.show();

  connect_wifi();
  
  // long t = millis();
  // int st = 0;
  // while(millis()-t < 4000){
  //   for(int l=0;l<NUM_LEDS;l++){
  //     if(l%10==st/10)       leds[l].setRGB(0,80,0);
  //     else                  leds[l] = 0x000000;
  //   }
  //   FastLED.show();
  //   if(++st>=10*10) st = 0;
  //   ArduinoOTA.handle();
  //   delay(5);
  // }
}

int x = 0;


void draw(){

  if(MODE.equals("full"))
    full();
  else if(MODE.equals("half"))
    half();
  else if(MODE.equals("line"))
    line();
  else if(MODE.equals("lines"))
    lines();
  else if(MODE.equals("hatch"))
    hatch();
  else if(MODE.equals("hatch2"))
    hatch2();
  else if(MODE.equals("scroll_lines"))
    scroll_lines();
  else if(MODE.equals("waves"))
    perc_a();
  else if(MODE.equals("stripes"))
    stripes();
  else if(MODE.equals("square"))
    square();
  else if(MODE.equals("triangle"))
    triangle();
  else if(MODE.equals("cross"))
    cross();
  else if(MODE.equals("arrow"))
    arrow();
  else if(MODE.equals("rain"))
    rain();
  
  else    full();

}

void loop() {
  
  draw();
  
  check_ota();
  check_server();
}