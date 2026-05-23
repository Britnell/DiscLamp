#pragma once
#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"
#include <math.h>

// Sphere radius slightly outside the disc so all pixels have a real pz
#define MOON_R 7.0f

// How far past pure-new we begin showing a crescent (radians).
// Smaller => deeper dark dip near new moon.
#define MOON_NEW_OFFSET  0.45f
// Full-moon overlap window half-width (radians). Larger => overlap starts
// earlier / ends later. Boundaries are continuous regardless.
#define MOON_FULL_OFFSET 0.40f
// How far the two halves diverge at the midpoint of the full-moon overlap
// (radians). Larger => darker slivers at the limbs, moon never looks fully
// lit. 0 => identical to single-draw (no effect).
#define MOON_FULL_SKEW   0.50f

// Cycle position in radians, 0..2*PI. Starts past the new-moon dead zone.
float moon_phase_angle = MOON_NEW_OFFSET;

// side: -1 = only px <= 0, +1 = only px >= 0, 0 = both halves
static void draw_moon(float phase, int8_t side) {
  float R = MOON_R;
  float sp = sinf(phase);
  float cp = cosf(phase);

  for(int l = 0; l < NUM_LEDS; l++) {
    float px = pixel[l].x;
    if(side < 0 && px > 0.0f) continue;
    if(side > 0 && px < 0.0f) continue;

    float py = pixel[l].y;
    float r2 = px*px + py*py;
    float pz = sqrtf(R*R - r2);

    float raw = px * sp - pz * cp;
    if(raw < 0.0f) raw = 0.0f;
    float intensity = raw / R;

    leds[l].setHSV(grad_hue(px, py), 250, (uint8_t)(intensity * 255.0f));
  }
}

void moon() {
  for(int l = 0; l < NUM_LEDS; l++) leds[l] = CRGB::Black;

  float t  = moon_phase_angle;
  float NO = MOON_NEW_OFFSET;
  float FO = MOON_FULL_OFFSET;

  if(t < M_PI - FO) {
    // pure waxing crescent/gibbous
    draw_moon(t, 0);
  }
  else if(t < M_PI + FO) {
    // Full-moon overlap. base sweeps π-FO -> π+FO so the boundaries match
    // the single-draw phase exactly (no jump). In the middle the two halves
    // diverge by `skew`, so the left half lags toward "still waxing" and
    // the right half leads toward "already waning" -- both showing a dark
    // sliver at their outer limb.
    float w    = (t - (M_PI - FO)) / (2.0f * FO);     // 0..1
    float base = (M_PI - FO) + 2.0f * FO * w;
    float skew = MOON_FULL_SKEW * sinf((float)M_PI * w);
    draw_moon(base - skew, -1);
    draw_moon(base + skew, +1);
  }
  else if(t < 2.0f * M_PI - NO) {
    // pure waning gibbous/crescent
    draw_moon(t, 0);
  }
  else {
    // new-moon overlap: left half is the fading waning crescent,
    // right half is the emerging waxing crescent of the next cycle
    float w  = (t - (2.0f * M_PI - NO)) / (2.0f * NO);  // 0..1 across the window
    float lp = (2.0f * M_PI - NO) + NO * w;             // -> 2*PI
    if(lp >= 2.0f * M_PI) lp -= 2.0f * M_PI;
    float rp = NO * w;                                  // 0 -> NO
    draw_moon(lp, -1);
    draw_moon(rp, +1);
  }

  FastLED.show();

  moon_phase_angle += 0.008f;
  // Wrap past the trailing edge of the new-moon overlap back to the
  // leading edge of waxing, so we skip pure-new entirely.
  if(moon_phase_angle > 2.0f * M_PI + NO) moon_phase_angle = NO;

  delay(30);
}
