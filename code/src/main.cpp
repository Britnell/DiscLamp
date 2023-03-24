#include <Arduino.h>
#include "pixel.h"
#include "connect.h"
#include "pattern.h"

void setup() {

  init_pixel();
  
  demo();
  leds[62].setRGB(0,0,20);
  leds[63].setRGB(0,0,20);
  leds[74].setRGB(0,0,20);
  leds[75].setRGB(0,0,250);
  leds[76].setRGB(0,0,20);
  leds[87].setRGB(0,0,20);
  leds[88].setRGB(0,0,20);
  FastLED.show();
  delay(100);

  connect_wifi();
  
}

void loop() {
  
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

  check_server();
  // put your main code here, to run repeatedly:
}