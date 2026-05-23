#ifndef DIGIT_H
#define DIGIT_H

#include "pixel.h"

typedef struct { int8_t r; int8_t c; } Cell;

// draw cells using hex grid coords — origin is hx/hy (0,0 = center)
void draw_cells(Cell* cells, int n, int8_t ohx, int8_t ohy, uint8_t h, uint8_t s, uint8_t v) {
  for (int i = 0; i < n; i++) {
    int8_t thx = ohx + cells[i].c * 2 + (cells[i].r % 2 != 0 ? 1 : 0);
    int8_t thy = ohy + cells[i].r;
    for (int l = 0; l < NUM_LEDS; l++) {
      if (pixel[l].hx == thx && pixel[l].hy == thy) {
        leds[l].setHSV(h, s, v); break;
      }
    }
  }
}


void draw8(int8_t hx, int8_t hy, uint8_t h) {
  Cell digit8[] = {
    {0,1},{0,2},
    {1,0},{1,2},
    {2,1},{2,2},
    {3,0},{3,2},
    {4,1},{4,2},
  };
  draw_cells(digit8, 10, hx, hy, h, 250, 255);
}
void draw2(int8_t hx, int8_t hy, uint8_t h) {
  Cell cells[] = {
    {0,1},{0,2},
    {1,0},{1,2},
    {2,1},{2,2},
    {4,1},{4,2},
    {3,0},{3,2},
  };
  draw_cells(cells, 10, hx, hy, h, 250, 255);
}
void draw3(int8_t hx, int8_t hy, uint8_t h) {
  Cell cells[] = { {0,1},{0,2}, {1,0}, {2,1},{2,2}, {3,0}, {4,1},{4,2} };
  draw_cells(cells, 8, hx, hy, h, 250, 255);
}
void draw1(int8_t hx, int8_t hy, uint8_t h) {
  Cell cells[] = { {0,0}, {1,0}, {2,1}, {3,0}, {4,0} };
  draw_cells(cells, 5, hx, hy, h, 250, 255);
}
void draw4(int8_t hx, int8_t hy, uint8_t h) {
  Cell cells[] = { {4,2},{4,0}, {3,2},{3,0}, {2,2},{2,1},{2,0}, {1,0}, {0,0} };
  draw_cells(cells, 9, hx, hy, h, 250, 255);
}
void draw5(int8_t hx, int8_t hy, uint8_t h) {
  Cell cells[] = { {4,2},{4,1}, {3,2}, {2,2},{2,1}, {1,0}, {0,2},{0,1} };
  draw_cells(cells, 7, hx, hy, h, 250, 255);
}

static int digit_frame = 0;

void digit_loop() {
  if (digit_frame == 0) {
    for (int l = 0; l < NUM_LEDS; l++) leds[l] = CRGB::Black;
    draw8(-8,-6,  hue);
    draw2(0 ,-6,  hue);
    // draw4(-8,2,  hue);
    // draw5(0 ,2,  hue);
    // draw4(5, 8,  hue);
    // draw5(5, 12, hue);
    FastLED.show();
  }
  digit_frame = (digit_frame + 1) % 10;
  delay(1000);
}

#endif
