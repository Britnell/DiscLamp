#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

uint8_t aut_initialised = 0;
uint8_t aut_state[NUM_LEDS];
uint8_t rule[3];

int autom_eras;


// 234 & 245 can get kinda stuck

void print_rules(){  
  p(rule[0]); p(" , ");
  p(rule[1]); p(" , ");
  p(rule[2]); p(" , ");
  pl();
}

void random_rules(){
  rule[0] = random(1,7);
  rule[1] = random(1,6);
  while(rule[0]==rule[1]){
    rule[1] = random(1,6);
  }
  rule[2] = random(1,6);
  while(rule[2]==rule[0]  || rule[2]==rule[1] ){
    rule[2] = random(1,6);
  }
  // print_rules();
}

void safe_rules(){
  rule[0] = 1;
  rule[1] = random(2,6);
  rule[2] = random(2,6);
  while(rule[1]==rule[2]){
    rule[2] = random(2,6);
  }
  // print_rules();
}

void autom_start(){
  for(int l=0;l<NUM_LEDS;l++){
    aut_state[l] = 0;
  }
  aut_state[62] = 1;
  aut_state[63] = 1;
  aut_state[74] = 1;
  aut_state[75] = 1;
  aut_state[76] = 1;
  aut_state[87] = 1;
  aut_state[88] = 1;
}

void init_autom(){
  autom_eras = 0;
  safe_rules();

  autom_start();
}


void find_neighbours(LED_STRUCT pix, int nbrs[6] ){
  // L & R
  if(pix.l>0){
    if(pix.row==pixel[pix.l-1].row){
      nbrs[0] = pix.l-1;
    }
  }
  if(pix.l<NUM_LEDS-1){
    if(pix.row==pixel[pix.l+1].row){
      nbrs[1] = pix.l+1;
    }
  }
  // diagonals
  nbrs[2] = find_pixel(pix.x+0.5,pix.y+DY);
  nbrs[3] = find_pixel(pix.x-0.5,pix.y+DY);
  nbrs[4] = find_pixel(pix.x+0.5,pix.y-DY);
  nbrs[5] = find_pixel(pix.x-0.5,pix.y-DY);
}

void update_autom(){
  uint8_t next [NUM_LEDS];

  for(int l=0;l<NUM_LEDS;l++){


    // find neighbors + count score
    int nbrs[6] = {-1,-1,-1,-1,-1,-1};
    int score = 0;
    find_neighbours(pixel[l],nbrs);
    for(int n=0;n<6;n++){
      if(nbrs[n]!=-1)
        score += aut_state[nbrs[n]];
    }
    // apply rule
    next[l] = 0;
    if(score>0){
      if(score==rule[0]  || score==rule[1]  || score==rule[2] )
        next[l] = 1;
    }
  }

  // Transfer state
  int total = 0;
  for(int l=0;l<NUM_LEDS;l++){
    aut_state[l] = next[l];
    total += next[l];
  }

  // check it snot dying out
  if(total==0) autom_start();
  else if(total<=12)   safe_rules();
}

void automaton(){
  if(!aut_initialised){
    init_autom();
    aut_initialised = 1;
  }


  // Paint
  for(int l=0;l<NUM_LEDS;l++){
    if(aut_state[l])    
    leds[l] += CRGB(0,0,0).setHSV(hue,250, 255).scale8(10);
    else    leds[l].nscale8(240);
  }
  FastLED.show();  
  delay(20);

  // automaton state
  if(millis()-timer > 1600){
    timer = millis();
    update_autom();

    if(autom_eras++ ==10){
      random_rules();
      autom_eras=0;
    }
  }
}
