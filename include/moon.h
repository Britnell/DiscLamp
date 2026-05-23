#pragma once
#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"
#include <math.h>

// Sphere radius slightly outside the disc so all pixels have a real pz
#define MOON_R 7.0f

// How early (radians before 2π) the next cycle's waxing crescent
// starts emerging on the right while the current waning crescent
// finishes fading on the left. Bigger => more overlap. Also the
// phase the next cycle continues at after the old one dies (so the
// next cycle starts already partway into waxing).
#define MOON_NEW_OFFSET  2.00f

// Base phase step per frame.
#define MOON_STEP_BASE   0.008f
// Half-width (radians) of the fast zone around π.
#define MOON_FAST_WIDTH  0.9f
#define MOON_STEP_FAST   0.032f

float moon_phase_angle = 1.0f;

// Only writes lit pixels. Multiple calls layer additively (later wins
// brighter pixel).
static void draw_moon(float phase) {
  float R = MOON_R;
  float sp = sinf(phase);
  float cp = cosf(phase);

  for(int l = 0; l < NUM_LEDS; l++) {
    float px = pixel[l].x;
    float py = pixel[l].y;
    float r2 = px*px + py*py;
    float pz = sqrtf(R*R - r2);

    float raw = px * sp - pz * cp;
    if(raw <= 0.0f) continue;

    uint8_t v = (uint8_t)((raw / R) * 255.0f);
    if(v > leds[l].getAverageLight()) {
      leds[l].setHSV(grad_hue(px, py), 250, v);
    }
  }
}

void moon() {
  for(int l = 0; l < NUM_LEDS; l++) leds[l] = CRGB::Black;

  float t = moon_phase_angle;

  draw_moon(t);
  if(t >= TWO_PI - MOON_NEW_OFFSET) {
    // overlap: also draw next cycle's emerging moon. draw_moon only
    // writes lit pixels so the two layer naturally without clobbering.
    draw_moon(t - (TWO_PI - MOON_NEW_OFFSET));
  }

  FastLED.show();

  // Step faster around full moon to shorten the full phase. Speed lerps
  // linearly from STEP_BASE at the edge of the fast zone to STEP_FAST at π.
  float step = MOON_STEP_BASE;
  // Symmetric distance from full moon (t = π). Ramps up approaching π
  // and back down after, then stays at BASE for the rest of the cycle
  // and the new cycle's wax until it nears π again.
  float d = fabsf(t - (float)M_PI);
  if(d < MOON_FAST_WIDTH) {
    float k = 1.0f - (d / MOON_FAST_WIDTH);
    step = MOON_STEP_BASE + (MOON_STEP_FAST - MOON_STEP_BASE) * k;
  }
  
  moon_phase_angle += step;

  // At t = 2π the old cycle is fully dead and the new cycle is at phase
  // MOON_NEW_OFFSET. Relabel so the new cycle continues from there.
  if(moon_phase_angle >= TWO_PI) moon_phase_angle = MOON_NEW_OFFSET + (moon_phase_angle - TWO_PI);

  delay(30);
}
