#ifndef DIGIT_H
#define DIGIT_H

#include "pixel.h"

typedef struct { int8_t x; int8_t y; } Cell;

// draw cells using hex grid coords — origin is hx/hy (0,0 = center)
void draw_cells(Cell* cells, int n, int8_t ohx, int8_t ohy, uint8_t h, uint8_t s, uint8_t v) {
  for (int i = 0; i < n; i++) {
    int8_t thx = ohx + cells[i].x * 2 + (cells[i].y % 2 != 0 ? 1 : 0);
    int8_t thy = ohy + cells[i].y;
    for (int l = 0; l < NUM_LEDS; l++) {
      if (pixel[l].hx == thx && pixel[l].hy == thy) {
        leds[l].setHSV(h, s, v); break;
      }
    }
  }
}


void draw8(int8_t hx, int8_t hy, uint8_t h) {
  Cell digit8[] = {
    {1,4},{2,4},
    {0,3},{2,3},
    {1,2},{2,2},
    {0,1},{2,1},
    {1,0},{2,0},
  };
  draw_cells(digit8, 10, hx, hy, h, 250, 255);
}
void draw2(int8_t hx, int8_t hy, uint8_t h) {
  Cell cells[] = {
    {1,4},{2,4},
    {0,3},{2,3},
    {2,2},
    {1,1},
    {1,0},{2,0},{3,0}
  };
  draw_cells(cells, 9, hx, hy, h, 250, 255);
}
void draw3(int8_t hx, int8_t hy, uint8_t h) {
  Cell cells[] = { 
    {1,4},{2,4},
    {2,3},
    {1,2},{2,2},
    {2,1},
    {1,0},{2,0},
  };
  draw_cells(cells, 8, hx, hy, h, 250, 255);
}
void draw4(int8_t hx, int8_t hy, uint8_t h) {
  Cell cells[] = { 
    {1,4},{3,4},
    {0,3},{2,3},
    {1,2},{2,2},{3,2},
    {2,1},
    {3,0}
  };
  draw_cells(cells, 9, hx, hy, h, 250, 255);
}
void draw5(int8_t hx, int8_t hy, uint8_t h) {
  Cell cells[] = { 
    {1,4},{2,4},
    {0,3},
    {1,2},{2,2},
    {2,1},
    {1,0},{2,0},
  };
  draw_cells(cells, 8, hx, hy, h, 250, 255);
}
void draw6(int8_t hx, int8_t hy, uint8_t h) {
  Cell cells[] = { 
    {1,4},{2,4},
    {0,3},
    {1,2},{2,2},
    {0,1},{2,1},
    {1,0},{2,0},
  };
  draw_cells(cells, 9, hx, hy, h, 250, 255);
}

void draw9(int8_t hx, int8_t hy, uint8_t h) {
  Cell cells[] = { 
    {1,4},{2,4},
    {0,3},{2,3},
    {1,2},{2,2},
    {2,1},
    {1,0},{2,0},
  };
  draw_cells(cells, 9, hx, hy, h, 250, 255);
}
void draw1(int8_t hx, int8_t hy, uint8_t h) {
  // Cell cells[] = { 
  //   {2,4}, 
  //   {2,3}, 
  //   {2,2}, 
  //   {2,1}, 
  //   {2,0} 
  // };
  Cell cells[] = { 
    {2,4}, 
    {1,3}, 
    {1,2}, 
    {1,1}, 
    {2,0} 
  };
  draw_cells(cells, 5, hx, hy, h, 250, 255);
}
void draw7(int8_t hx, int8_t hy, uint8_t h) {
  Cell cells[] = { 
    {1,4},{2,4},{3,4},
    {2,3}, 
    {2,2}, 
    {1,1}, 
    {1,0} 
  };
  draw_cells(cells, 7, hx, hy, h, 250, 255);
}

static int digit_frame = 0;

void digit_loop() {
  if (digit_frame == 0) {
    for (int l = 0; l < NUM_LEDS; l++) leds[l] = CRGB::Black;
    draw1(-7,1,  hue);
    draw7( 1,1,  hue);
    draw9(-7,-5,  hue);
    draw9( 1,-5,  hue);
    FastLED.show();
  }
  digit_frame = (digit_frame + 1) % 10;
  delay(1000);
}

#endif
