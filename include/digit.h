#ifndef DIGIT_H
#define DIGIT_H

#include "pixel.h"

typedef struct { int8_t r; int8_t c; } Cell;

// find LED index by (row, col-within-row), returns -1 if not found
int find_ij(int row, int col) {
  for (int l = 0; l < NUM_LEDS; l++) {
    if (pixel[l].row == row && pixel[l].row_i == col)
      return l;
  }
  return -1;
}

// draw an array of cells at origin (or, oc), with given hue
void draw_cells(Cell* cells, int n, int or_, int oc, uint8_t h, uint8_t s, uint8_t v) {
  for (int i = 0; i < n; i++) {
    int l = find_ij(or_ + cells[i].r, oc + cells[i].c);
    if (l >= 0) leds[l].setHSV(h, s, v);
  }
}

Cell digit8[] = {
  {0,1},{0,2},
  {1,0},{1,2},
  {2,1},{2,2},
  {3,0},{3,2},
  {4,1},{4,2},
};

Cell digit1[] = {
  {0,0},
  {1,0},
  {2,1},
  {3,0},
  {4,0}
};

Cell digit2[] = {
  {4,1},{4,0},
  {3,0},
  {2,1},{2,0},
  {1,2},
  {0,2},{0,1},
};

Cell digit3[] = {
  {4,1},{4,0},
  {3,0},
  {2,1},{2,0},
  {1,0},
  {0,1},{0,0},
};

Cell digit4[] = {
  {4,2},{4,0},
  {3,2},{3,0},
  {2,2},{2,1},{2,0},
  {1,0},
  {0,0},
};

Cell digit5[] = {
  {4,2},{4,1},
  {3,2},
  {2,2},{2,1},
  {1,0},
  {0,2},{0,1},
};

void draw8(int row, int col, uint8_t h) {
  draw_cells(digit8, 10, row, col, h, 250, 255);
}
void draw1(int row, int col, uint8_t h) {
  draw_cells(digit1, 5, row, col, h, 250, 255);
}
void draw2(int row, int col, uint8_t h) {
  draw_cells(digit2, 7, row, col, h, 250, 255);
}
void draw3(int row, int col, uint8_t h) {
  draw_cells(digit3, 7, row, col, h, 250, 255);
}
void draw4(int row, int col, uint8_t h) {
  draw_cells(digit4, 9, row, col, h, 250, 255);
}
void draw5(int row, int col, uint8_t h) {
  draw_cells(digit5, 7, row, col, h, 250, 255);
}

static int digit_frame = 0;

void digit_loop() {
  if (digit_frame == 0) {
    for (int l = 0; l < NUM_LEDS; l++) leds[l] = CRGB::Black;
    // draw1(3, 0,  hue);
    draw8(2,2, hue);
    // draw3(5, 4,  hue);
    // draw4(5, 8,  hue);
    // draw5(5, 12, hue);
    FastLED.show();
  }
  digit_frame = (digit_frame + 1) % 10;
  delay(1000);
}

#endif
