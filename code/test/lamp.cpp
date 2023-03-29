#include <Arduino.h>
#include "pixel.h"
#include "serial.h"
#include "pattern.h"
#include "automaton.h"
#include "rain.h"
#include "hues.h"
#include "zig.h"
// #include "fast.h"

void setup() {

  Serial.begin(115200);

  init_pixel();
  
  demo();
  Serial.println(" ready");
}

void loop() {
  read_serial();
  
  if(mode.equals("full"))   full();
  else if(mode.equals("half"))    half();
  else if(mode.equals("square"))    square();
  else if(mode.equals("triangle"))    triangle();
  else if(mode.equals("circle"))    circle();
  
  else if(mode.equals("lines"))   lines();
  else if(mode.equals("hatch"))   hatch();
  else if(mode.equals("hatch2"))    hatch2();
  else if(mode.equals("arrow"))   arrow();
  else if(mode.equals("trif"))   trif();
  
  else if(mode.equals("bar"))    line();
  else if(mode.equals("stripes"))   stripes();
  else if(mode.equals("scroll"))    scroll_lines();
  else if(mode.equals("waves"))   waves();
  else if(mode.equals("zig"))   zig();
  
  else if(mode.equals("rain"))    rain();
  else if(mode.equals("hues"))    hues();
  else if(mode.equals("autom"))   automaton();
  
  // else if(mode.equals("cross"))   cross();
  // else if(mode.equals("count"))    //   count();
  // else if(mode.equals("fast"))   fast();
  
  else square();

}