#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

static const uint8_t FIRE_STEPS    = 52; 
static const uint8_t FIRE_SPAWN    = 25;  // % chance to seed an ember in bottom rows
static const uint8_t FIRE_COOL_MIN = 5;   // cooling as heat rises
static const uint8_t FIRE_COOL_MAX = 50;  
static const uint8_t FIRE_BOT_COOL = 25; 
static const uint8_t FIRE_HUE_MAX  = 20; 

static uint8_t fh[NUM_LEDS];       // current sim state (target)
static uint8_t fh_prev[NUM_LEDS];  // previous sim state (interp from)
static uint8_t fire_rdy = 0;
static uint8_t fire_tick = 0;

void fire_sim_step() {
  memcpy(fh_prev, fh, NUM_LEDS);

  // Seed bottom 2 rows sparsely — ~35% of pixels lit to create distinct streaks
  for (int l = 0; l < NUM_LEDS; l++) {
    if (pixel[l].row <= 1) {
      if (random(100) < FIRE_SPAWN)
        fh[l] = 170 + random(86);
      else
        fh[l] = fh[l] > FIRE_BOT_COOL ? fh[l] - FIRE_BOT_COOL : 0;  // let unlit pixels cool
    }
  }

  // Push heat upward: each cell splits its heat between its two upper hex
  // neighbours by a random ratio (p / 100-p). Upper cells accumulate from
  // both their below-children, then cool. Total heat per row is conserved
  // before cooling, so streaks neither vanish nor explode — they wander.
  uint16_t acc[NUM_LEDS];
  memset(acc, 0, sizeof(acc));
  for (int l = 0; l < NUM_LEDS; l++) {
    if (fh[l] == 0) continue;
    int r = pixel[l].row;
    float x = pixel[l].x;

    int ul = -1, ur = -1;
    for (int n = 0; n < NUM_LEDS; n++) {
      if (pixel[n].row != r + 1) continue;
      float dx = pixel[n].x - x;
      if (dx > -0.75f && dx < -0.25f) ul = n;
      else if (dx > 0.25f && dx < 0.75f) ur = n;
    }

    int p = random(101);
    if (ul >= 0) acc[ul] += (uint16_t)fh[l] * p / 100;
    if (ur >= 0) acc[ur] += (uint16_t)fh[l] * (100 - p) / 100;
  }

  uint8_t nh[NUM_LEDS];
  for (int l = 0; l < NUM_LEDS; l++) {
    if (pixel[l].row <= 1) { nh[l] = fh[l]; continue; }
    int v = (int)acc[l] - random(FIRE_COOL_MIN, FIRE_COOL_MAX);
    if (v < 0) v = 0;
    if (v > 255) v = 255;
    nh[l] = (uint8_t)v;
  }
  memcpy(fh, nh, NUM_LEDS);
}

void fire() {
  if (!fire_rdy) {
    memset(fh, 0, NUM_LEDS);
    memset(fh_prev, 0, NUM_LEDS);
    fire_rdy = 1;

    // pre-run the sim so fire doesn't start blank
    for (int i = 0; i < 6; i++)
        fire_sim_step();
  }

  if (++fire_tick >= FIRE_STEPS) {
    fire_tick = 0;
    fire_sim_step();
  }

  // Interpolate between previous and current sim state for smooth fading
  uint16_t t = (uint16_t)((fire_tick * 256) / FIRE_STEPS);  // 0..255

  // Fire palette: black -> red -> orange — no green, just a hint of yellow at the hottest
  for (int l = 0; l < NUM_LEDS; l++) {
    uint8_t h = (uint8_t)(((uint16_t)fh_prev[l] * (256 - t) + (uint16_t)fh[l] * t) >> 8);
    uint8_t hv, sat, val;
    if (h < 85) {
      hv = 0; sat = 255; val = map(h, 0, 84, 0, 255);
    } else if (h < 170) {
      hv = map(h, 85, 169, 0, 20); sat = 255; val = 255;
    } else {
      hv  = map(h, 170, 255, 18, FIRE_HUE_MAX);  // orange → yellow-orange (no green)
      sat = 255;
      val = 255;
    }
    leds[l].setHSV(hv, sat, val);
  }

  FastLED.show();
  delay(30);
}
