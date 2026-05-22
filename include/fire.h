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

  // Seed bottom 3 rows with hot, flickering heat
  for (int l = 0; l < NUM_LEDS; l++) {
    if (pixel[l].row >= N_ROWS - 3)
      fh[l] = 148 + random(108);
  }

  // Diffuse heat upward: each LED averages hex-neighbours in the row below,
  // with a small random lateral drift to break up straight columns
  uint8_t nh[NUM_LEDS];
  for (int l = 0; l < NUM_LEDS; l++) {
    int r = pixel[l].row;
    if (r >= N_ROWS - 3) {
      nh[l] = fh[l];
      continue;
    }

    float lx = pixel[l].x + (random(3) - 1) * 0.55f;
    uint32_t sum = 0;
    int cnt = 0;

    for (int n = 0; n < NUM_LEDS; n++) {
      if (pixel[n].row == r + 1) {
        float dx = lx - pixel[n].x;
        if (dx * dx < 1.3f) {
          sum += fh[n];
          cnt++;
        }
      }
    }

    int v = cnt > 0 ? (int)(sum / cnt) - 5 - random(20) : 0;
    nh[l] = v < 0 ? 0 : (uint8_t)v;
  }
  memcpy(fh, nh, NUM_LEDS);

  // Map heat 0-255 to fire palette: black -> red -> orange -> yellow-white
  for (int l = 0; l < NUM_LEDS; l++) {
    uint8_t h = fh[l];
    uint8_t hv, sat, val;
    if (h < 85) {
      hv = 0; sat = 255; val = map(h, 0, 84, 0, 255);
    } else if (h < 170) {
      hv = map(h, 85, 169, 0, 28); sat = 255; val = 255;
    } else {
      hv  = map(h, 170, 255, 28, 48);
      sat = map(h, 170, 255, 255, 80);
      val = 255;
    }
    leds[l].setHSV(hv, sat, val);
  }

  FastLED.show();
  delay(35);
}
