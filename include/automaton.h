#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

uint8_t aut_initialised = 0;
uint8_t aut_state[NUM_LEDS];
uint8_t aut_prev[NUM_LEDS];
uint8_t rule[3];

int autom_eras;
int autom_hist[5] = {1,2,3,4,5};

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


void autom_check(int total){
  // track history
  for(int h=4;h>0;h--){
    autom_hist[h] = autom_hist[h-1];
  }
  autom_hist[0] = total;

  // check if dying out
  if(total<6) autom_start();
  else if(total<=12)   safe_rules();
  // catch stuck / alternating patterns
  else if(autom_hist[0]==autom_hist[1] && autom_hist[1]==autom_hist[2] && autom_hist[2]==autom_hist[3])
    random_rules();
  else if(autom_hist[0]==autom_hist[2] && autom_hist[1]==autom_hist[3])
    random_rules();
}

int autom_step(){
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
  return total;
}

void init_autom(){
  autom_eras = 0;
  safe_rules();

  autom_start();

  // pre-run a couple of gens so it doesn't start as a tiny blob
  for(int i=0;i<3;i++) autom_step();

  // fresh fade-in from black, and don't let a stale `timer`
  // (shared with other patterns) tick the first gen instantly
  memcpy(aut_prev, aut_state, NUM_LEDS);
  timer = millis();
}

void automaton(){
  if(!aut_initialised){
    init_autom();
    aut_initialised = 1;
  }

  // Paint — fade progress derived from time, not frame count
  // first half of FADE_MS: OFF->colB, second half: colB->colA
  #define FADE_MS 4000 // keep <= gen length (4000) so fades finish before the next state change
  uint16_t elapsed = millis() - timer; // timer reset each gen, so this is gen age
  if(elapsed > FADE_MS) elapsed = FADE_MS;
  CRGB col_a; col_a.setHSV(hue, 250, 255);        // the one colour everything settles on
  CRGB col_b; col_b.setHSV((uint8_t)hue_a, 250, 255); // flash colour, only for newly-on pixels
  for(int l=0;l<NUM_LEDS;l++){
    if(aut_state[l] && aut_prev[l])  { leds[l] = col_a; }
    else if(aut_state[l]) {
      if(elapsed < FADE_MS/2){
        // step 1: fade up in colB
        leds[l] = col_b;
        leds[l].nscale8( (uint32_t)elapsed * 510UL / FADE_MS );
      } else {
        // step 2: cross-fade colB -> colA
        leds[l] = blend(col_b, col_a, (uint32_t)(elapsed - FADE_MS/2) * 510UL / FADE_MS );
      }
    }
    else leds[l].nscale8(246);
  }
  FastLED.show();  
  delay(10);

  // automaton state
  if(millis()-timer > 4000){
    timer = millis();
    memcpy(aut_prev, aut_state, NUM_LEDS);
    int count = autom_step();
    autom_check(count);

    if(autom_eras++ == 10){
      random_rules();
      autom_eras=0;
    }
  }
}
