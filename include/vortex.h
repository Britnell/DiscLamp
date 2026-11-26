#ifndef VORTEX_H
#define VORTEX_H

#include <Arduino.h>
#include <math.h>
#include "pattern.h"
#include "pixel.h"

// * * * vortex — rotating spiral galaxy * * *
// Soft Archimedean spiral arms wind out of a glowing core and slowly turn.
//   phase  = N * (angle - rot) + twist * r
//   arm    = (0.5 + 0.5 cos phase) ^ sharp
// twist "breathes" between a tight coil and nearly straight spokes, the spin
// eases toward random targets (sometimes reversing) and the arm count
// crossfades between 2 and 3 every now and then.

// ------------------------- tunables -------------------------
// disc radius used for normalising (outermost pixel is ~6.24)
#define VORTEX_R          6.2f
// arm counts we alternate between (crossfaded)
#define VORTEX_ARMS_LO    2
#define VORTEX_ARMS_HI    3
// frames between arm-count changes (random in range) and crossfade length
#define VORTEX_ARMS_MIN   1500
#define VORTEX_ARMS_MAX   3000
#define VORTEX_ARMS_FADE  150
// arm shape exponent: higher = thinner arms, wider dark gaps
#define VORTEX_SHARP      2.5f
// arms fade in from the core between these radii (hides aliasing at centre)
#define VORTEX_ARM_IN0    0.4f
#define VORTEX_ARM_IN1    2.0f
// arm brightness lost toward the rim (0 = none, 1 = black at rim)
#define VORTEX_RIM_FADE   0.3f
// twist in rad per unit radius: breathes between MIN..MAX (sine)
// above ~1.6 the arms start to alias into noise near the rim
#define VORTEX_TWIST_MIN  0.3f
#define VORTEX_TWIST_MAX  1.5f
// breathing period in frames (~33 fps -> 1300 = ~40 s)
#define VORTEX_TWIST_PERIOD 1300.0f
// extra random drift added to twist (+-)
#define VORTEX_TWIST_DRIFT 0.15f
// rotation speed in rad / frame (max target), at param = 50
#define VORTEX_SPIN_MAX   0.010f
#define VORTEX_SPIN_MIN   0.003f
// chance (0..100) that a new spin target reverses direction
#define VORTEX_REVERSE    25
// easing of spin / drift toward target per frame (smaller = smoother)
#define VORTEX_EASE       0.004f
// frames a spin target is held before choosing a new one
#define VORTEX_HOLD_MIN   200
#define VORTEX_HOLD_MAX   500
// core glow: strength, radius (gaussian sigma), desaturation toward white
#define VORTEX_CORE       0.85f
#define VORTEX_CORE_SIGMA 1.3f
#define VORTEX_CORE_DESAT 90
// faint dust glow over the whole disc, fading to the rim
#define VORTEX_FLOOR      0.05f
// hue shift centre -> rim as multiple of (hue_a - hue)
#define VORTEX_HUE_SPREAD 1.0f
// frame delay in ms
#define VORTEX_DELAY      30
// ------------------------------------------------------------

static float vx_r[NUM_LEDS];
static float vx_a[NUM_LEDS];
static uint8_t vx_initialised = 0;

static float vx_rot = 0.0f;          // current rotation angle
static float vx_spin = 0.0f;         // current angular velocity
static float vx_spin_target = 0.0f;
static int   vx_hold = 0;            // frames until new spin target

static float vx_breath = 0.0f;       // twist breathing phase
static float vx_drift = 0.0f;
static float vx_drift_target = 0.0f;

static uint8_t vx_n_from = VORTEX_ARMS_LO;
static uint8_t vx_n_to = VORTEX_ARMS_LO;
static int     vx_fade = 0;          // crossfade frames remaining (0 = idle)
static int     vx_arms_wait = 0;     // frames until next arm change

static float vortex_frand(float lo, float hi) {
  return lo + (hi - lo) * (float)random(0, 1001) / 1000.0f;
}

void init_vortex() {
  if (vx_initialised) return;
  vx_initialised = 1;

  for (int l = 0; l < NUM_LEDS; l++) {
    float x = pixel[l].x;
    float y = pixel[l].y;
    vx_r[l] = sqrtf(x * x + y * y);
    vx_a[l] = atan2f(y, x);
  }
  vx_rot = vortex_frand(0.0f, TWO_PI);
  vx_spin = VORTEX_SPIN_MIN;
  vx_spin_target = VORTEX_SPIN_MIN;
  vx_breath = vortex_frand(0.0f, TWO_PI);
  vx_arms_wait = random(VORTEX_ARMS_MIN, VORTEX_ARMS_MAX);
}

void vortex_tick() {
  // spin: ease toward a target, pick a new one now and then
  if (vx_hold > 0) {
    vx_hold--;
  } else {
    vx_hold = random(VORTEX_HOLD_MIN, VORTEX_HOLD_MAX);
    float s = vortex_frand(VORTEX_SPIN_MIN, VORTEX_SPIN_MAX);
    // keep the current direction mostly, reverse occasionally
    float dir = (vx_spin_target < 0.0f) ? -1.0f : 1.0f;
    if (random(100) < VORTEX_REVERSE) dir = -dir;
    vx_spin_target = dir * s;
    vx_drift_target = vortex_frand(-VORTEX_TWIST_DRIFT, VORTEX_TWIST_DRIFT);
  }
  float speed_k = linear(param, 0, 100, 0.4f, 1.6f);
  vx_spin += (vx_spin_target * speed_k - vx_spin) * VORTEX_EASE;
  vx_drift += (vx_drift_target - vx_drift) * VORTEX_EASE;

  vx_rot += vx_spin;
  if (vx_rot > TWO_PI) vx_rot -= TWO_PI;
  if (vx_rot < 0.0f) vx_rot += TWO_PI;

  vx_breath += TWO_PI / VORTEX_TWIST_PERIOD;
  if (vx_breath > TWO_PI) vx_breath -= TWO_PI;

  // arm count crossfade
  if (vx_fade > 0) {
    vx_fade--;
    if (vx_fade == 0) vx_n_from = vx_n_to;
  } else if (--vx_arms_wait <= 0) {
    vx_arms_wait = random(VORTEX_ARMS_MIN, VORTEX_ARMS_MAX);
    vx_n_to = (vx_n_from == VORTEX_ARMS_LO) ? VORTEX_ARMS_HI : VORTEX_ARMS_LO;
    vx_fade = VORTEX_ARMS_FADE;
  }
}

static inline float vortex_arm(uint8_t n, float a, float tr, float sharp) {
  float c = 0.5f + 0.5f * cosf(n * (a - vx_rot) + tr);
  return powf(c, sharp);
}

void vortex() {
  init_vortex();
  vortex_tick();

  // 0 = loose spokes, 1 = tight coil
  float coil = 0.5f - 0.5f * cosf(vx_breath);
  float twist = VORTEX_TWIST_MIN + (VORTEX_TWIST_MAX - VORTEX_TWIST_MIN) * coil + vx_drift;
  if (twist < 0.1f) twist = 0.1f;

  // crossfade weight toward vx_n_to, smoothstepped
  float mix = 0.0f;
  if (vx_fade > 0) {
    float t = 1.0f - (float)vx_fade / VORTEX_ARMS_FADE;
    mix = t * t * (3.0f - 2.0f * t);
  }

  // core glows a touch bigger / brighter while the arms are coiled tight
  float sigma = VORTEX_CORE_SIGMA * (0.85f + 0.3f * coil);
  float inv_s2 = 1.0f / (sigma * sigma);

  int8_t hue_d = (int8_t)(uint8_t)(hue_a - hue);   // signed, wraps correctly
  int8_t core_d = (int8_t)(uint8_t)(hue_b - hue);

  for (int l = 0; l < NUM_LEDS; l++) {
    float r = vx_r[l];
    float a = vx_a[l];
    float rn = r / VORTEX_R;
    if (rn > 1.0f) rn = 1.0f;
    float tr = twist * r;

    float arm = vortex_arm(vx_n_from, a, tr, VORTEX_SHARP);
    if (mix > 0.0f) {
      float arm2 = vortex_arm(vx_n_to, a, tr, VORTEX_SHARP);
      arm += (arm2 - arm) * mix;
    }

    // fade arms in from the core, and slightly out toward the rim
    float t = (r - VORTEX_ARM_IN0) / (VORTEX_ARM_IN1 - VORTEX_ARM_IN0);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    arm *= t * t * (3.0f - 2.0f * t);
    arm *= 1.0f - VORTEX_RIM_FADE * rn;

    float core = VORTEX_CORE * expf(-r * r * inv_s2);

    float v = arm + core + VORTEX_FLOOR * (1.0f - rn);
    if (v > 1.0f) v = 1.0f;
    int val = (int)(v * 255.0f);
    if (invert) val = 255 - val;

    float cw = core / VORTEX_CORE;   // 0..1 core weight
    float hf = hue_d * VORTEX_HUE_SPREAD * rn * (1.0f - cw) + core_d * cw;
    uint8_t h = (uint8_t)((int16_t)hue + (int16_t)hf);
    uint8_t sat = 250 - (uint8_t)(VORTEX_CORE_DESAT * cw);

    leds[l].setHSV(h, sat, val);
  }
  FastLED.show();

  delay(VORTEX_DELAY);
}

#endif
