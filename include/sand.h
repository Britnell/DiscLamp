#ifndef SAND_H
#define SAND_H

#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

// * * * sand — tilting glass of coloured grains * * *
// Cellular sand on the hex grid. Gravity is one of the 6 hex directions; grains
// fall one cell per step, otherwise slide down-left / down-right, and settle into
// heaps. Every while the lamp is "tilted" (gravity turns 60 deg, sometimes flips
// 180 deg) and the heap avalanches into a new pile, layers folding into each other.
// Grain count is constant. Grains glide between cells (prev/next blend).

// ------------------------- tunables -------------------------
// fill fraction of the disc in percent: SAND_FILL_MIN .. SAND_FILL_MAX from param 0..100
#define SAND_FILL_MIN 35
#define SAND_FILL_MAX 62
// frames per simulation step (grain moves one cell per step, glides over these frames)
#define SAND_SUB 4
// chance (0..255) an eligible grain actually slides sideways this step (falling is always)
#define SAND_SLIDE_P 215
// rest time between tilts in ms (after the heap has settled)
#define SAND_REST_MIN 14000
#define SAND_REST_MAX 30000
// chance (0..255) a tilt is a 180 deg flip instead of +-60 deg
#define SAND_FLIP_P 60
// thickness of a colour layer in grains
#define SAND_LAYER_MIN 7
#define SAND_LAYER_MAX 18
// brightness of a resting grain / of a moving grain (0..255)
#define SAND_V_REST 185
#define SAND_V_MOVE 255
// heat of a moving grain decays by this much per frame (cools to rest brightness)
#define SAND_COOL 28
// brightness of the empty glass (0..255); invert: lit background
#define SAND_V_EMPTY 4
#define SAND_V_INVERT 190
// shimmer: chance (0..255) per frame a resting grain twinkles
#define SAND_SHIMMER_P 40
// param change (0..100) that triggers a re-seed at the next tilt
#define SAND_RESEED_DELTA 15
// frame delay in ms
#define SAND_DELAY 14
// ------------------------------------------------------------

#define SAND_WALL 255
#define SAND_NONE 255

// grain byte: 0 empty, else 1 + band(0..3) + 4 * shade(0..3)
static uint8_t sand_nb[NUM_LEDS][6];   // neighbour table, dir 0..5 counter-clockwise from +x
static uint8_t sand_grain[NUM_LEDS];
static uint8_t sand_from[NUM_LEDS];    // cell the grain came from this step (SAND_NONE = didn't move)
static uint8_t sand_heat[NUM_LEDS];    // 0..255 glow of a moving grain
static uint8_t sand_order[NUM_LEDS];   // cells sorted lowest-first along gravity
static uint8_t sand_g = 5;             // gravity direction
static uint8_t sand_sub = 0;
static bool    sand_ready = false;
static bool    sand_settled = false;
static uint32_t sand_next_tilt = 0;
static uint8_t sand_seed_param = 50;

// hex step per direction in (hx, hy)
static const int8_t SAND_DHX[6] = {  2,  1, -1, -2, -1,  1 };
static const int8_t SAND_DHY[6] = {  0,  1,  1,  0, -1, -1 };

static int sand_proj(uint8_t c) {
  return pixel[c].hx * SAND_DHX[sand_g] + 3 * pixel[c].hy * SAND_DHY[sand_g];
}

// order[] = cells with the largest projection on gravity first ("lowest" cell first)
static void sand_sort() {
  int key[NUM_LEDS];
  for (int i = 0; i < NUM_LEDS; i++) {
    int k = sand_proj(i);
    int j = i - 1;
    while (j >= 0 && key[j] < k) { key[j + 1] = key[j]; sand_order[j + 1] = sand_order[j]; j--; }
    key[j + 1] = k;
    sand_order[j + 1] = i;
  }
}

static void sand_build_nb() {
  for (int i = 0; i < NUM_LEDS; i++)
    for (int d = 0; d < 6; d++) {
      sand_nb[i][d] = SAND_WALL;
      for (int j = 0; j < NUM_LEDS; j++)
        if (pixel[j].hx == pixel[i].hx + SAND_DHX[d] && pixel[j].hy == pixel[i].hy + SAND_DHY[d]) {
          sand_nb[i][d] = j;
          break;
        }
    }
}

// fill the highest cells (opposite of gravity) with layered grains -> they fall right away
static void sand_seed() {
  int fill = SAND_FILL_MIN + (int)param * (SAND_FILL_MAX - SAND_FILL_MIN) / 100;
  int n = NUM_LEDS * fill / 100;
  memset(sand_grain, 0, sizeof(sand_grain));
  memset(sand_from, SAND_NONE, sizeof(sand_from));
  memset(sand_heat, 0, sizeof(sand_heat));
  sand_sort();
  uint8_t band = random(4);
  int left = random(SAND_LAYER_MIN, SAND_LAYER_MAX + 1);
  for (int k = 0; k < n; k++) {
    if (left-- <= 0) {
      band = (band + 1 + random(3)) & 3;
      left = random(SAND_LAYER_MIN, SAND_LAYER_MAX + 1);
    }
    sand_grain[sand_order[NUM_LEDS - 1 - k]] = 1 + band + 4 * random(4);
  }
  sand_seed_param = param;
  sand_settled = false;
  sand_sub = SAND_SUB - 1;
}

static void sand_init() {
  sand_build_nb();
  sand_g = 5;
  sand_seed();
  sand_ready = true;
}

static inline void sand_move(uint8_t c, uint8_t n) {
  sand_grain[n] = sand_grain[c];
  sand_grain[c] = 0;
  sand_heat[c] = 0;
  sand_heat[n] = 255;
  sand_from[n] = c;
}

// one simulation step; returns number of grains that could move (0 = heap is stable)
static int sand_step() {
  memset(sand_from, SAND_NONE, sizeof(sand_from));
  int eligible = 0;
  const uint8_t ga = (sand_g + 1) % 6, gb = (sand_g + 5) % 6;
  for (int k = 0; k < NUM_LEDS; k++) {
    uint8_t c = sand_order[k];
    if (!sand_grain[c]) continue;
    uint8_t n = sand_nb[c][sand_g];
    if (n != SAND_WALL && !sand_grain[n]) {
      eligible++;
      sand_move(c, n);
      continue;
    }
    uint8_t a = sand_nb[c][ga], b = sand_nb[c][gb];
    bool ea = a != SAND_WALL && !sand_grain[a];
    bool eb = b != SAND_WALL && !sand_grain[b];
    if (!ea && !eb) continue;
    eligible++;
    if (random8() >= SAND_SLIDE_P) continue;
    if (ea && eb) sand_move(c, random8() & 1 ? a : b);
    else          sand_move(c, ea ? a : b);
  }
  return eligible;
}

static void sand_tilt() {
  if (abs((int)param - (int)sand_seed_param) >= SAND_RESEED_DELTA) {
    sand_g = random(6);
    sand_seed();
    return;
  }
  if (random8() < SAND_FLIP_P) sand_g = (sand_g + 3) % 6;
  else                         sand_g = (sand_g + (random8() & 1 ? 1 : 5)) % 6;
  sand_sort();
  sand_settled = false;
  sand_sub = SAND_SUB - 1;
}

// per-frame render accumulators
static uint16_t sand_acc_v[NUM_LEDS];
static uint16_t sand_acc_w[NUM_LEDS];
static uint8_t  sand_top_w[NUM_LEDS];
static uint8_t  sand_top_g[NUM_LEDS];

static inline void sand_draw(uint8_t cell, uint8_t g, uint8_t w, uint8_t v) {
  sand_acc_v[cell] += (uint16_t)w * v / 255;
  sand_acc_w[cell] += w;
  if (w > sand_top_w[cell]) { sand_top_w[cell] = w; sand_top_g[cell] = g; }
}

void sand() {
  if (!sand_ready) sand_init();

  // simulation
  if (++sand_sub >= SAND_SUB) {
    sand_sub = 0;
    if (!sand_settled) {
      if (sand_step() == 0) {
        sand_settled = true;
        sand_next_tilt = millis() + random(SAND_REST_MIN, SAND_REST_MAX + 1);
      }
    } else if ((int32_t)(millis() - sand_next_tilt) >= 0) {
      sand_tilt();
    }
  }
  uint8_t p = (uint8_t)(255 * (sand_sub + 1) / SAND_SUB);   // glide progress 0..255

  // shimmer on the resting heap
  if (sand_settled && random8() < SAND_SHIMMER_P) {
    uint8_t c = random(NUM_LEDS);
    if (sand_grain[c] && sand_heat[c] < 110) sand_heat[c] = 110;
  }

  memset(sand_acc_v, 0, sizeof(sand_acc_v));
  memset(sand_acc_w, 0, sizeof(sand_acc_w));
  memset(sand_top_w, 0, sizeof(sand_top_w));
  for (int c = 0; c < NUM_LEDS; c++) {
    uint8_t g = sand_grain[c];
    if (!g) continue;
    uint8_t v = SAND_V_REST + (uint16_t)(SAND_V_MOVE - SAND_V_REST) * sand_heat[c] / 255;
    if (sand_from[c] != SAND_NONE) {
      sand_draw(c, g, p, v);
      sand_draw(sand_from[c], g, 255 - p, v);
    } else {
      sand_draw(c, g, 255, v);
    }
    sand_heat[c] = sand_heat[c] > SAND_COOL ? sand_heat[c] - SAND_COOL : 0;
  }

  const uint8_t bandhue[3] = { (uint8_t)hue, (uint8_t)hue_a, (uint8_t)hue_b };
  for (int l = 0; l < NUM_LEDS; l++) {
    uint16_t cover = sand_acc_w[l] > 255 ? 255 : sand_acc_w[l];
    uint8_t h = (uint8_t)hue, s = 250;
    if (sand_top_w[l]) {
      uint8_t g = sand_top_g[l] - 1;
      uint8_t band = g & 3, shade = g >> 2;
      h = band < 3 ? bandhue[band] : (uint8_t)hue;
      s = band < 3 ? 255 - shade * 16 : 70 + shade * 14;      // band 3 = pale sand
      h += (shade - 1) * 2;
    }
    uint16_t v;
    if (!invert) {
      v = sand_acc_v[l] + SAND_V_EMPTY * (255 - cover) / 255;
    } else {
      // grains are dark specks in a lit background
      v = SAND_V_INVERT * (255 - cover) / 255 + (sand_acc_v[l] >> 3);
      if (cover < 128) s = 250;
    }
    leds[l].setHSV(h, s, v > 255 ? 255 : v);
  }

  FastLED.show();
  delay(SAND_DELAY);
}

#endif
