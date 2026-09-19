#ifndef GRID_H
#define GRID_H

#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

// * * * params * * *
#define GRID_SPACING_X 6      // hx is doubled coords (in-row step=2): 8 = every 4th visible led
#define GRID_SPACING_Y 3      // rows
#define GRID_HOLD_MS   2000   // hold time before moving
#define GRID_FADE_MS   2000   // crossfade duration
#define GRID_STEP_HX   1      // x drift per step (doubled coords: 1 = slight right)
#define GRID_STEP_HY   -1     // y drift per step (rows down)
#define GRID_Y_SKEW     1      // x offset per y-band: hex rows have no straight-down pixel, odd band-counts shift 1 over

static uint32_t grid_t0 = 0;
static uint8_t  grid_phase = 0;     // 0 = hold, 1 = fade
static uint16_t grid_step = 0;

static int grid_floordiv(int a, int b){
  int q = a / b;
  if((a % b != 0) && ((a < 0) != (b < 0))) q--;
  return q;
}

static uint8_t grid_dot(int hx, int hy, uint16_t s){
  int x = hx - GRID_STEP_HX * (int)s;
  int y = hy - GRID_STEP_HY * (int)s;
  x -= grid_floordiv(y, GRID_SPACING_Y) * GRID_Y_SKEW;   // skew x by band index
  int m;
  if(x>=0) m = x % GRID_SPACING_X; else m = (GRID_SPACING_X - ((-x) % GRID_SPACING_X)) % GRID_SPACING_X;
  if(m) return 0;
  if(y>=0) m = y % GRID_SPACING_Y; else m = (GRID_SPACING_Y - ((-y) % GRID_SPACING_Y)) % GRID_SPACING_Y;
  return m==0;
}

// moving grid: hold, then crossfade to next skewed position, repeat

void grid(){

  uint32_t now = millis();
  if(grid_phase == 0){
    if(now - grid_t0 >= GRID_HOLD_MS){
      grid_phase = 1;
      grid_t0 = now;
    }
  } else {
    if(now - grid_t0 >= GRID_FADE_MS){
      grid_phase = 0;
      grid_t0 = now;
      grid_step++;
    }
  }

  float fade = (grid_phase == 1) ? (float)(now - grid_t0) / GRID_FADE_MS : 0.0;
  if(fade > 1.0) fade = 1.0;

  for(int l=0;l<NUM_LEDS;l++){
    uint8_t onA = grid_dot(pixel[l].hx, pixel[l].hy, grid_step);
    uint8_t onB = grid_dot(pixel[l].hx, pixel[l].hy, grid_step + 1);

    int vA = onA ? 255 : 0;
    int vB = onB ? 255 : 0;
    int val = vA + (int)(fade * (vB - vA));

    if(invert) val = 255 - val;

    leds[l].setHSV( grad_hue(pixel[l].x, pixel[l].y) ,250, val );
  }
  FastLED.show();

  delay(20);
}

#endif
