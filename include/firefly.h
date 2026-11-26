#ifndef FIREFLY_H
#define FIREFLY_H

#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

// * * * firefly — fireflies that learn to blink in sync * * *
// Pulse-coupled oscillators (Mirollo-Strogatz style). Each firefly charges a
// phase, flashes at 1 and nudges nearby fireflies forward. The coupling reach
// and strength ramp up over the cycle: random -> local clusters -> travelling
// waves -> whole meadow flashing as one. After it has held a while a
// disturbance blacks the meadow out, scatters the phases and starts over.

// ------------------------- tunables -------------------------
// number of fireflies
#define FF_N 26
// natural period spread of the fireflies, seconds
#define FF_PERIOD_MIN 1.75f
#define FF_PERIOD_MAX 2.25f
// coupling at full strength: fraction of the remaining phase skipped per
// neighbour flash (before distance weighting)
#define FF_EPS 0.30f
// coupling radius (units, pixel pitch = 1): starts local, ends disc-wide
#define FF_R_START 2.6f
#define FF_R_END   11.0f
// seconds for coupling to ramp from 0 to full at param 50
// (param 0 => x2 slower, param 100 => x0.67 faster)
#define FF_RAMP_S 38.0f
// phase below which a firefly ignores neighbours (refractory, avoids echoes)
#define FF_REFRACT 0.20f
// flash shape: rise time and decay time-constant, ms
#define FF_ATTACK_MS 60.0f
#define FF_DECAY_MS  200.0f
// share of a flash that spills onto the 6 neighbouring pixels
#define FF_HALO 0.30f
// whole-disc wash when many fireflies flash together (0..1)
#define FF_WASH 0.30f
// order parameter above which the meadow counts as synchronised,
// and how long it must stay that way before the disturbance, ms
#define FF_SYNC_LEVEL 0.80f
#define FF_HOLD_MS 14000
// give up waiting and scatter anyway after this long, ms
#define FF_TIMEOUT_MS 120000
// fireflies that fly to a new spot on scatter
#define FF_RELOCATE 7
// blackout fade out / in during the disturbance, ms
#define FF_SCATTER_OUT_MS 500
#define FF_SCATTER_IN_MS  700
// peak saturation drop (warm white at the flash peak)
#define FF_SAT_WHITE 130
// frame delay in ms
#define FF_DELAY 12
// ------------------------------------------------------------

#define FF_RUN     0
#define FF_SCATTER 1

static uint8_t  ff_pix[FF_N];      // pixel index
static float    ff_th[FF_N];       // phase 0..1
static float    ff_per[FF_N];      // period, seconds
static uint8_t  ff_hsel[FF_N];     // hue source
static uint32_t ff_last[FF_N];     // millis of last flash
static bool     ff_inited = false;
static uint8_t  ff_state = FF_RUN;
static uint32_t ff_t_prev = 0;     // last frame
static uint32_t ff_t_cycle = 0;    // cycle start
static uint32_t ff_t_state = 0;    // scatter start
static uint32_t ff_sync_since = 0; // 0 = not currently synced
static uint32_t ff_t_order = 0;
static float    ff_coupling = 0;   // 0..1
static float    ff_order = 0;
static bool     ff_relocated = false;

static float ff_randf() { return random(0, 10001) / 10000.0f; }

static bool ff_taken(uint8_t l, int upto) {
  for (int i = 0; i < upto; i++) if (ff_pix[i] == l) return true;
  return false;
}

static void ff_reset_cycle(uint32_t now) {
  for (int i = 0; i < FF_N; i++) {
    ff_th[i] = ff_randf();
    ff_last[i] = now - 5000;      // no leftover glow
  }
  ff_t_cycle = now;
  ff_sync_since = 0;
  ff_coupling = 0;
  ff_order = 0;
}

static void ff_init(uint32_t now) {
  for (int i = 0; i < FF_N; i++) {
    uint8_t l;
    do { l = random(0, NUM_LEDS); } while (ff_taken(l, i));
    ff_pix[i] = l;
    ff_per[i] = FF_PERIOD_MIN + (FF_PERIOD_MAX - FF_PERIOD_MIN) * ff_randf();
    ff_hsel[i] = random(0, 3);
  }
  ff_reset_cycle(now);
  ff_t_prev = now;
  ff_inited = true;
}

static void ff_relocate() {
  for (int k = 0; k < FF_RELOCATE; k++) {
    int i = random(0, FF_N);
    uint8_t l;
    do { l = random(0, NUM_LEDS); } while (ff_taken(l, FF_N));
    ff_pix[i] = l;
  }
}

// advance the oscillators by dt seconds
static void ff_step(float dt, uint32_t now) {
  float rampS = FF_RAMP_S * 100.0f / (50.0f + param);
  float c = (now - ff_t_cycle) / (rampS * 1000.0f);
  ff_coupling = c > 1.0f ? 1.0f : c;
  float R   = FF_R_START + ff_coupling * (FF_R_END - FF_R_START);
  float eps = FF_EPS * ff_coupling;

  bool fired[FF_N];
  bool any = false;
  for (int i = 0; i < FF_N; i++) {
    fired[i] = false;
    ff_th[i] += dt / ff_per[i];
    if (ff_th[i] >= 1.0f) {
      ff_th[i] = 0;
      ff_last[i] = now;
      fired[i] = true;
      any = true;
    }
  }
  if (!any || eps <= 0) return;

  for (int i = 0; i < FF_N; i++) {
    if (!fired[i]) continue;
    float xi = pixel[ff_pix[i]].x, yi = pixel[ff_pix[i]].y;
    for (int j = 0; j < FF_N; j++) {
      if (j == i || ff_th[j] < FF_REFRACT) continue;
      float dx = pixel[ff_pix[j]].x - xi, dy = pixel[ff_pix[j]].y - yi;
      float d2 = dx * dx + dy * dy;
      if (d2 >= R * R) continue;
      float w = 1.0f - 0.5f * sqrtf(d2) / R;
      ff_th[j] += eps * w * (1.0f - ff_th[j]);
      if (ff_th[j] >= 1.0f) ff_th[j] = 0.9999f;   // fires next frame
    }
  }
}

// Kuramoto order parameter |sum e^{2 pi i theta}| / N
static float ff_calc_order() {
  float sx = 0, sy = 0;
  for (int i = 0; i < FF_N; i++) {
    float a = 6.2831853f * ff_th[i];
    sx += cosf(a);
    sy += sinf(a);
  }
  return sqrtf(sx * sx + sy * sy) / FF_N;
}

void firefly() {
  uint32_t now = millis();
  if (!ff_inited) ff_init(now);
  uint32_t dtms = now - ff_t_prev;
  if (dtms > 100) dtms = 100;          // survive mode switches
  ff_t_prev = now;
  float dt = dtms / 1000.0f;

  // ---- state machine ----
  float fade = 1.0f;
  if (ff_state == FF_RUN) {
    ff_step(dt, now);
    if (now - ff_t_order >= 100) {
      ff_t_order = now;
      ff_order = ff_calc_order();
      if (ff_order > FF_SYNC_LEVEL) {
        if (!ff_sync_since) ff_sync_since = now;
      } else ff_sync_since = 0;
    }
    bool synced = ff_sync_since && (now - ff_sync_since > FF_HOLD_MS);
    if (synced || now - ff_t_cycle > FF_TIMEOUT_MS) {
      ff_state = FF_SCATTER;
      ff_t_state = now;
      ff_relocated = false;
    }
  } else {
    uint32_t e = now - ff_t_state;
    if (e < FF_SCATTER_OUT_MS) {
      fade = 1.0f - (float)e / FF_SCATTER_OUT_MS;
      ff_step(dt, now);                 // keep flashing while it darkens
    } else {
      if (!ff_relocated) {
        ff_relocated = true;
        ff_relocate();
        ff_reset_cycle(now);
      }
      e -= FF_SCATTER_OUT_MS;
      if (e >= FF_SCATTER_IN_MS) ff_state = FF_RUN;
      else fade = (float)e / FF_SCATTER_IN_MS;
    }
  }

  // ---- render ----
  static float acc[NUM_LEDS];
  static int8_t hoff[NUM_LEDS];
  float wash_n = 0;
  for (int l = 0; l < NUM_LEDS; l++) { acc[l] = 0; hoff[l] = 0; }

  for (int i = 0; i < FF_N; i++) {
    uint32_t since = now - ff_last[i];
    if (since > 1500) continue;
    float b = since < FF_ATTACK_MS ? since / FF_ATTACK_MS
                                   : expf(-(since - FF_ATTACK_MS) / FF_DECAY_MS);
    if (b < 0.02f) continue;
    if (b > 0.5f) wash_n += 1.0f;
    int8_t ho = ff_hsel[i] == 1 ? (int8_t)(hue_a - hue)
              : ff_hsel[i] == 2 ? (int8_t)(hue_b - hue) : 0;
    uint8_t c = ff_pix[i];
    float cx = pixel[c].x, cy = pixel[c].y;
    for (int l = 0; l < NUM_LEDS; l++) {
      float dx = pixel[l].x - cx;
      float dy = pixel[l].y - cy;
      float d2 = dx * dx + dy * dy;
      if (d2 > 1.7f) continue;          // self + 6 neighbours (distance 1)
      float v = (l == c) ? b : b * FF_HALO;
      if (v > acc[l] * 0.5f) hoff[l] = ho;
      acc[l] += v;
    }
  }

  float wf = wash_n / FF_N;
  float wash = FF_WASH * wf * wf;

  for (int l = 0; l < NUM_LEDS; l++) {
    float x = pixel[l].x, y = pixel[l].y;
    float a = acc[l] + wash;
    if (a > 1.0f) a = 1.0f;

    // faint breathing "grass", a touch brighter toward the bottom
    float grass = 5.0f + 3.0f * (sin8((now >> 4) + l * 29) / 255.0f)
                + (y < 0 ? -y * 0.8f : 0);

    uint8_t h = grad_hue(x, y) + hoff[l];
    uint8_t s = 250 - (uint8_t)(a * a * FF_SAT_WHITE);
    float vf;
    if (invert) {
      vf = 170.0f * (1.0f - a) + grass;
      s = 250;
    } else {
      vf = a * 255.0f + grass * (1.0f - a);
    }
    if (vf > 255.0f) vf = 255.0f;
    leds[l].setHSV(h, s, (uint8_t)(vf * fade));
  }
  FastLED.show();

  delay(FF_DELAY);
}

#endif
