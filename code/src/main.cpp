#include <Arduino.h>
#include "pixel.h"
// #include "connect.h"
#include "pattern.h"
#include "serial.h"

void setup() {

  Serial.begin(115200);

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

  Serial.println(" ready");
}

void loop() {
  read_serial();
  
  if(mode.equals("full"))
    full();
  else if(mode.equals("half"))
    half();
  else if(mode.equals("line"))
    line();
  else if(mode.equals("lines"))
    lines();
  else if(mode.equals("hatch"))
    hatch();
  else if(mode.equals("hatch2"))
    hatch2();
  else if(mode.equals("scroll_lines"))
    scroll_lines();
  else if(mode.equals("waves"))
    perc_a();
  else if(mode.equals("stripes"))
    stripes();
  else if(mode.equals("square"))
    square();
  else if(mode.equals("triangle"))
    triangle();
  else if(mode.equals("cross"))
    cross();
  else if(mode.equals("arrow"))
    arrow();
  else if(mode.equals("rain"))
    rain();
  else if(mode.equals("count"))
    count();
  else if(mode.equals("autom"))
    automaton();
  
  else square();

  // check_server();
}