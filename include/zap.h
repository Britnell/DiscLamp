#ifndef ZAP_H
#define ZAP_H

#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

// * * * zap — laser lines crossing * * *
// Beams shoot in from the rim with a hot, whitish head, leave a full-length
// line that holds, then fade (sometimes the tail chases the head off the disc).
// Where beams cross the pixel flares toward white and sparks now and then.

// ------------------------- tunables -------------------------
// max beams alive at once
#define ZAP_BEAMS 4
// beam half-width (units, pixel pitch = 1). smoothstep falloff,
// so ~1.3 => 2-3 px wide line with soft edges
#define ZAP_W_MIN 1.1f
#define ZAP_W_MAX 1.5f
// head speed along the beam, units per frame
#define ZAP_SPEED_MIN 0.45f
#define ZAP_SPEED_MAX 0.95f
// length of the hot (bright, desaturated) band behind the head
#define ZAP_HEAD_LEN 1.6f
// soft leading edge in front of the head
#define ZAP_HEAD_SOFT 0.5f
// brightness of the line behind the head (0..1), head peaks at 1
#define ZAP_TRAIL 0.62f
// frames a beam holds full length before fading
#define ZAP_HOLD_MIN 12
#define ZAP_HOLD_MAX 45
// per-frame brightness multiplier while fading
#define ZAP_FADE 0.88f
// chance (0..255) the tail chases the head out while fading
#define ZAP_CHASE_P 110
// chance (0..255) a beam strobes on ignition, and for how many frames
#define ZAP_FLICKER_P 70
#define ZAP_FLICKER_FRAMES 9
// chance (0..255) a beam snaps to one of the 3 hex axes (crisp on the grid)
#define ZAP_AXIS_P 170
// frames between spawns (scaled by param: higher param = busier)
#define ZAP_GAP_MIN 6
#define ZAP_GAP_MAX 40
// crossing: chance (0..255) per frame a crossing pixel sparks full white
#define ZAP_SPARK_P 45
// lowest saturation at the hottest point (head / crossing)
#define ZAP_SAT_WHITE 60
// afterglow decay per frame (scale8, 255 = never fades)
#define ZAP_AFTERGLOW 205
// radius of the disc used to compute where a beam enters / leaves
#define ZAP_R 6.6f
// frame delay in ms
#define ZAP_DELAY 18
// ------------------------------------------------------------

#define ZAP_DEAD   0
#define ZAP_FIRING 1
#define ZAP_HOLD   2
#define ZAP_FADING 3

typedef struct {
  uint8_t state;
  uint8_t hsel;     // 0 = hue, 1 = hue_a, 2 = hue_b offset
  uint8_t flicker;  // remaining strobe frames
  uint8_t chase;    // tail chases the head when fading
  uint8_t timer;    // hold frames left
  float ang;        // line direction angle
  float dx, dy;     // unit direction (head travels along +dir)
  float nx, ny;     // unit normal
  float d;          // perpendicular offset from centre
  float w;          // half width
  float head;       // head position along the line
  float tail;       // tail position (only moves when chasing)
  float end;        // where the beam leaves the disc
  float speed;
  float level;      // 0..1 brightness multiplier
} ZapBeam;

static ZapBeam zap_beam[ZAP_BEAMS];
static uint8_t zap_glow_v[NUM_LEDS];
static uint8_t zap_glow_h[NUM_LEDS];
static int     zap_gap = 0;

static float zap_randf(float a, float b) {
  return a + (b - a) * (random(0, 1001) / 1000.0f);
}

// smallest difference between two line angles (lines are mod π)
static float zap_ang_diff(float a, float b) {
  float d = fmodf(fabsf(a - b), (float)PI);
  return d > (float)HALF_PI ? (float)PI - d : d;
}

static float zap_pick_angle() {
  if (random8() < ZAP_AXIS_P) return random(0, 3) * (float)PI / 3.0f;
  return zap_randf(0.0f, (float)PI);
}

static void zap_spawn(ZapBeam *b) {
  // try a few angles, keep the one most different from live beams -> crossings
  float best = 0, best_score = -1;
  for (int k = 0; k < 4; k++) {
    float a = zap_pick_angle();
    float score = (float)PI;
    for (int i = 0; i < ZAP_BEAMS; i++) {
      if (zap_beam[i].state == ZAP_DEAD || &zap_beam[i] == b) continue;
      float df = zap_ang_diff(a, zap_beam[i].ang);
      if (df < score) score = df;
    }
    if (score > best_score) { best_score = score; best = a; }
  }
  b->ang = best;
  float a = best + (random8() & 1 ? (float)PI : 0.0f);   // fire from either end
  b->dx = cosf(a);
  b->dy = sinf(a);
  b->nx = -b->dy;
  b->ny = b->dx;

  // offsets in half lattice-row steps: on a row (1 bright px + soft neighbours)
  // or between rows (2 px) — keeps axis beams crisp
  b->d = random(-9, 10) * (DY * 0.5f);
  b->w = zap_randf(ZAP_W_MIN, ZAP_W_MAX);

  float L = sqrtf(ZAP_R * ZAP_R - b->d * b->d);
  b->head  = -L - ZAP_HEAD_SOFT;
  b->tail  = -L - 2.0f;
  b->end   = L + ZAP_HEAD_LEN;
  b->speed = zap_randf(ZAP_SPEED_MIN, ZAP_SPEED_MAX);

  b->hsel    = random(0, 3);
  b->flicker = random8() < ZAP_FLICKER_P ? ZAP_FLICKER_FRAMES : 0;
  b->chase   = random8() < ZAP_CHASE_P;
  b->timer   = random(ZAP_HOLD_MIN, ZAP_HOLD_MAX + 1);
  b->level   = 1.0f;
  b->state   = ZAP_FIRING;
}

static void zap_step(ZapBeam *b) {
  switch (b->state) {
    case ZAP_FIRING:
      b->head += b->speed;
      if (b->flicker) b->flicker--;
      if (b->head >= b->end) b->state = ZAP_HOLD;
      break;
    case ZAP_HOLD:
      if (b->timer) b->timer--;
      else          b->state = ZAP_FADING;
      break;
    case ZAP_FADING:
      b->level *= ZAP_FADE;
      if (b->chase) b->tail += b->speed * 1.2f;
      if (b->level < 0.03f || b->tail >= b->end) b->state = ZAP_DEAD;
      break;
  }
}

static void zap_tick() {
  for (int i = 0; i < ZAP_BEAMS; i++) zap_step(&zap_beam[i]);

  if (zap_gap > 0) { zap_gap--; return; }
  for (int i = 0; i < ZAP_BEAMS; i++) {
    if (zap_beam[i].state != ZAP_DEAD) continue;
    zap_spawn(&zap_beam[i]);
    // param 0..100 -> gap x1.5 .. x0.5
    int g = random(ZAP_GAP_MIN, ZAP_GAP_MAX + 1);
    zap_gap = g * (150 - param) / 100;
    return;
  }
}

void zap() {
  zap_tick();

  // per-beam strobe level for this frame (flicker on ignition)
  float lvl[ZAP_BEAMS];
  int8_t hoff[ZAP_BEAMS];
  for (int i = 0; i < ZAP_BEAMS; i++) {
    ZapBeam *b = &zap_beam[i];
    lvl[i] = b->level;
    if (b->flicker) lvl[i] *= (random8() < 120) ? 0.12f : 1.0f;
    hoff[i] = b->hsel == 1 ? (int8_t)(hue_a - hue)
            : b->hsel == 2 ? (int8_t)(hue_b - hue) : 0;
  }

  for (int l = 0; l < NUM_LEDS; l++) {
    float x = pixel[l].x;
    float y = pixel[l].y;

    float sum = 0, white = 0, v1 = 0, v2 = 0;
    int top = -1;

    for (int i = 0; i < ZAP_BEAMS; i++) {
      ZapBeam *b = &zap_beam[i];
      if (b->state == ZAP_DEAD) continue;

      float dist = fabsf(b->nx * x + b->ny * y - b->d);
      if (dist >= b->w) continue;
      float t = b->dx * x + b->dy * y;

      // soft leading edge at the head, soft cut at the tail
      float behind = b->head - t;
      if (behind <= -ZAP_HEAD_SOFT) continue;
      float along = behind < 0 ? 1.0f + behind / ZAP_HEAD_SOFT : 1.0f;
      float past = t - b->tail;
      if (past <= 0) continue;
      if (past < 1.0f) along *= past;

      // hot band right behind a moving head
      float hot = 0;
      if (b->state == ZAP_FIRING && behind < ZAP_HEAD_LEN)
        hot = behind < 0 ? 1.0f : 1.0f - behind / ZAP_HEAD_LEN;

      float k = 1.0f - dist / b->w;
      float prof = k * k * (3.0f - 2.0f * k);   // smoothstep -> soft edges

      float v = prof * along * lvl[i] * (ZAP_TRAIL + (1.0f - ZAP_TRAIL) * hot);
      sum += v;
      float wh = hot * prof * lvl[i];
      if (wh > white) white = wh;

      if (v > v1)      { v2 = v1; v1 = v; top = i; }
      else if (v > v2) { v2 = v; }
    }

    // crossing: second beam strong enough -> flare toward white, maybe spark
    if (v2 > 0.2f) {
      float c = v2 / ZAP_TRAIL;
      if (c > 1.0f) c = 1.0f;
      if (c * 0.85f > white) white = c * 0.85f;
      sum += 0.35f * c;
      if (c > 0.5f && random8() < ZAP_SPARK_P) { white = 1.0f; sum = 1.0f; }
    }
    if (sum > 1.0f) sum = 1.0f;
    if (white > 1.0f) white = 1.0f;

    uint8_t fresh = (uint8_t)(sum * 255.0f);
    uint8_t g = scale8(zap_glow_v[l], ZAP_AFTERGLOW);
    uint8_t h, s, v;

    if (fresh > 0 && fresh >= g) {
      h = grad_hue(x, y) + hoff[top];
      s = 250 - (uint8_t)(white * (250 - ZAP_SAT_WHITE));
      v = fresh;
      zap_glow_v[l] = fresh;
      zap_glow_h[l] = h;
    } else {
      // afterglow: cooling, fully saturated ghost of earlier beams
      zap_glow_v[l] = g;
      h = (g == 0 && invert) ? grad_hue(x, y) : zap_glow_h[l];
      s = 250;
      v = g;
    }

    if (invert) { v = 255 - v; s = 250; }
    leds[l].setHSV(h, s, v);
  }
  FastLED.show();

  delay(ZAP_DELAY);
}

#endif
