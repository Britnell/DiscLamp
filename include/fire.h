#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

static uint8_t fh[NUM_LEDS];
static uint8_t fire_rdy = 0;

void fire() {
  if (!fire_rdy) {
    memset(fh, 0, NUM_LEDS);
    fire_rdy = 1;
  }

  // Seed bottom 2 rows sparsely — ~35% of pixels lit to create distinct streaks
  for (int l = 0; l < NUM_LEDS; l++) {
    if (pixel[l].row >= N_ROWS - 2) {
      if (random(100) < 35)
        fh[l] = 170 + random(86);
      else
        fh[l] = fh[l] > 8 ? fh[l] - 8 : 0;  // let unlit pixels cool
    }
  }

  // Diffuse upward: each LED inherits from the single closest neighbour in the
  // row below, shifted by a random lateral drift. Single-neighbour sampling
  // (not averaging) preserves gaps → thin streaks. High-variance cooling means
  // lucky streaks survive higher, unlucky ones die early.
  uint8_t nh[NUM_LEDS];
  for (int l = 0; l < NUM_LEDS; l++) {
    int r = pixel[l].row;
    if (r >= N_ROWS - 2) {
      nh[l] = fh[l];
      continue;
    }

    float lx = pixel[l].x + (random(5) - 2) * 0.6f;

    uint8_t best_heat = 0;
    float   best_dx   = 99.0f;
    for (int n = 0; n < NUM_LEDS; n++) {
      if (pixel[n].row == r + 1) {
        float dx = fabsf(lx - pixel[n].x);
        if (dx < best_dx) {
          best_dx   = dx;
          best_heat = fh[n];
        }
      }
    }

    // Range 6–60: wide variance — lucky streaks climb much higher, unlucky ones die fast
    int v = (int)best_heat - 6 - random(55);
    nh[l] = v < 0 ? 0 : (uint8_t)v;
  }
  memcpy(fh, nh, NUM_LEDS);

  // Fire palette: black -> red -> orange -> yellow (no white)
  for (int l = 0; l < NUM_LEDS; l++) {
    uint8_t h = fh[l];
    uint8_t hv, sat, val;
    if (h < 85) {
      hv = 0; sat = 255; val = map(h, 0, 84, 0, 255);
    } else if (h < 170) {
      hv = map(h, 85, 169, 0, 36); sat = 255; val = 255;
    } else {
      hv  = map(h, 170, 255, 36, 60);  // orange to yellow
      sat = 255;                         // stay saturated, no white
      val = 255;
    }
    leds[l].setHSV(hv, sat, val);
  }

  FastLED.show();
  delay(110);
}
