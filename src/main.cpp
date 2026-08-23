#include <Arduino.h>

#include "serial.h"
#include "pixel.h"
#include "pattern.h"

#include "automaton.h"
#include "rain.h"
#include "hues.h"
#include "zig.h"
#include "clock.h"
#include "binaryclock.h"
#include "snake.h"
// #include "fast.h"
#include "moon.h"
#include "fire.h"
#include "digit.h"
#include "ring.h"
#include "secrings.h"

void setup() {

  Serial.begin(115200);

  init_pixel();
  snake_reshuffle();
  rain_randomize();
  demo();
  Serial.println(" ready");
}

void loop() {
  read_serial();
  grad_tick();
  if(mode != "clock" && mode != "binaryclock" && mode != "secrings" && mode != "autom") {
    if(millis() - timer >= 1000){
      timer = millis();
      tick();
    }
  }
  
  if(mode.equals("full"))   full();
  else if(mode.equals("half"))    half();
  else if(mode.equals("square"))    square();
  else if(mode.equals("triangle"))    triangle();
  else if(mode.equals("circle"))    circle();
  
  else if(mode.equals("lines"))   lines();

  else if(mode.equals("scroll_lines"))    scroll_lines();
  else if(mode.equals("waves"))   waves();
  else if(mode.equals("zig"))   zig();
  
  else if(mode.equals("rain"))    rain();
  else if(mode.equals("hues"))    hues();
  else if(mode.equals("autom"))   automaton();
  else if(mode.equals("clock"))   clock_loop();
  else if(mode.equals("binaryclock"))   binaryclock_loop();
  else if(mode.equals("snake"))   snake_loop();
  else if(mode.equals("moon"))    moon();
  else if(mode.equals("fire"))    fire();
  else if(mode.equals("digit"))   digit_loop();
  else if(mode.equals("secrings"))  secrings_loop();

  // else if(mode.equals("cross"))   cross();
  // else if(mode.equals("count"))    //   count();
  // else if(mode.equals("fast"))   fast();
  
  else square();

}