#ifndef RAY_H
#define RAY_H

#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

#include "orbit.h"

// ------------------------- tunables -------------------------
// width = distance we dim pixels, falling off quadratically
#define RAY_W 1.0f
// orbit steps per frame - higher = faster drifting centre
#define RAY_SPEED 2
// frame delay in ms
#define RAY_DELAY 60
// where the centre starts
#define RAY_START_X 2.3f
#define RAY_START_Y 3.6f
// ------------------------------------------------------------

// centre point of the ray burst, drifting around like the hues orbits
Orbit ray_centre;
uint8_t ray_initialised = 0;

void init_ray() {
  if (ray_initialised) return;
  ray_initialised = 1;

  ray_centre.x = RAY_START_X;
  ray_centre.y = RAY_START_Y;
  ray_centre.vx = 0;
  ray_centre.vy = 0;

  float ang = random_angle();
  ray_centre.ax = cos(ang);
  ray_centre.ay = sin(ang);
}

// brightness 0..255 of the line through the centre with unit normal (nx, ny).
// falloff is squared -> exponential-feeling fade as a line sweeps a pixel
int ray_line_val(float nx, float ny, float x, float y) {
  float d = fabsf(nx * (x - ray_centre.x) + ny * (y - ray_centre.y));
  if (d >= RAY_W) return 0;
  float t = 1.0f - d / RAY_W;
  return (int)(255.0f * t * t);
}

// 3 lines through the centre along the hex grid axes:
//   horizontal              -> normal ( 0   ,  1  )
//   diagonal up-right       -> normal ( DY  , -0.5)   line dir ( 0.5, DY)
//   diagonal up-left        -> normal ( DY  ,  0.5)   line dir (-0.5, DY)
// (normals are unit length since DY = sqrt(3)/2)
void ray() {
  init_ray();

  for (int i = 0; i < RAY_SPEED; i++)
    step_orbit(&ray_centre);

  for (int l = 0; l < NUM_LEDS; l++) {
    float x = pixel[l].x;
    float y = pixel[l].y;

    int b = ray_line_val(0.0f, 1.0f, x, y);
    int b2 = ray_line_val(DY, -0.5f, x, y);
    if (b2 > b) b = b2;
    int b3 = ray_line_val(DY, 0.5f, x, y);
    if (b3 > b) b = b3;

    int val = invert ? 255 - b : b;

    leds[l].setHSV(grad_hue(x, y), 250, val);
  }
  FastLED.show();

  delay(RAY_DELAY);
}

#endif
