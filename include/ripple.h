#ifndef RIPPLE_H
#define RIPPLE_H

#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

// * * * ripple — raindrops on a pond * * *
//
// Drops land at random spots. Each one flashes a small splash, then sends out
// a short packet of concentric rings (a few wavelengths long) that travels
// outward, dimming and spreading. Packets from several drops are summed as a
// signed height field, so crests that meet add up brighter and crests meeting
// troughs cancel. Rings also reflect once off the rim (image source) for extra
// interference. A dim, slowly breathing "water" base keeps the disc alive.
//
// Wave maths uses FastLED's integer cos16 and culls pixels outside each ring
// packet's annulus with squared-distance checks, so most pixel/drop pairs cost
// only a few float multiplies.

// ------------------------- tunables -------------------------
// max simultaneous drops (each also gets a rim reflection)
#define RIPPLE_MAX_DROPS 5
// wavelength range of the rings (pixel spacing ~1; keep >= ~2.2 to avoid aliasing)
#define RIPPLE_LAMBDA_MIN 2.3f
#define RIPPLE_LAMBDA_MAX 2.9f
// ring front speed in units/s (disc radius ~6)
#define RIPPLE_SPEED_MIN 1.6f
#define RIPPLE_SPEED_MAX 2.2f
// ring packet: lead crest sits half a wavelength behind the front, followed by
// a tail RIPPLE_RINGS wavelengths long (trailing crests ~0.5, then fade)
#define RIPPLE_RINGS 2.0f
// amplitude decay per second (exp(-k*age))
#define RIPPLE_DAMP 0.3f
// geometric spreading: amp /= sqrt(1 + R / RIPPLE_SPREAD_R)
#define RIPPLE_SPREAD_R 3.0f
// drop strength range, and soft-clip knee for overlapping crests: h / (1 + k|h|)
#define RIPPLE_AMP_MIN 1.0f
#define RIPPLE_AMP_MAX 1.4f
#define RIPPLE_CLIP 0.35f
// big drop: 1 in N chance, amplitude / wavelength / speed / ring-count multipliers
#define RIPPLE_BIG_CHANCE 6
#define RIPPLE_BIG_AMP 1.4f
#define RIPPLE_BIG_LAMBDA 1.15f
#define RIPPLE_BIG_SPEED 0.8f
#define RIPPLE_BIG_RINGS 1.6f
// drops land within this radius of the centre
#define RIPPLE_LAND_R 4.5f
// disc radius used for the rim reflection, and reflected amplitude (0 = off)
#define RIPPLE_DISC_R 6.3f
#define RIPPLE_REFLECT 0.4f
// splash: soft dot at the impact point lasting RIPPLE_SPLASH_T seconds
#define RIPPLE_SPLASH_T 0.35f
#define RIPPLE_SPLASH_SIGMA2 0.8f
// mean seconds between drops, at param = 100 .. param = 0 (param = rain intensity)
#define RIPPLE_GAP_FAST 0.6f
#define RIPPLE_GAP_SLOW 3.0f
// brightness: water base level, its slow shimmer, crest/trough gains
#define RIPPLE_BASE 38
#define RIPPLE_BASE_WOBBLE 14
#define RIPPLE_GAIN_UP 230.0f
#define RIPPLE_GAIN_DOWN 38.0f
// hue nudge at full crest (+ toward hue_a) / trough (- toward hue_b)
#define RIPPLE_HUE_SHIFT 10.0f
// crests desaturate toward white by up to this much (glint)
#define RIPPLE_GLINT 70.0f
// frame delay in ms
#define RIPPLE_DELAY 20
// ------------------------------------------------------------

typedef struct {
  float cx, cy;     // impact point
  float rx, ry;     // image source behind the rim (for the reflection)
  float age;        // seconds since impact
  float A;          // initial amplitude
  float inv_lambda; // 1 / wavelength
  float v;          // front speed
  float lead;       // distance of the lead crest behind the front (lambda / 2)
  float len;        // total packet length behind the front
  float inv_tail;   // 1 / (len - lead)
  uint8_t alive;
} RippleDrop;

static RippleDrop ripple_drops[RIPPLE_MAX_DROPS];
static uint32_t ripple_last_ms = 0;
static float ripple_next_drop = 0.0f;   // seconds until the next drop
static uint16_t ripple_noise_t = 0;

static float ripple_frand(float a, float b) {
  return a + (b - a) * (random(0, 10001) / 10000.0f);
}

static float ripple_gap() {
  float mean = linear(param, 0, 100, RIPPLE_GAP_SLOW, RIPPLE_GAP_FAST);
  // poisson-ish: exponential spread, but never back-to-back
  float u = ripple_frand(0.05f, 1.0f);
  float g = -mean * logf(u);
  return g < 0.15f ? 0.15f : g;
}

static void ripple_spawn() {
  for (int i = 0; i < RIPPLE_MAX_DROPS; i++) {
    RippleDrop *d = &ripple_drops[i];
    if (d->alive) continue;

    // uniform over the landing disc
    float r = RIPPLE_LAND_R * sqrtf(ripple_frand(0.0f, 1.0f));
    float a = ripple_frand(0.0f, TWO_PI);
    d->cx = r * cosf(a);
    d->cy = r * sinf(a);

    // image source mirrored through the rim: distance 2R - |c| on the same ray
    float s = (r > 0.05f) ? (2.0f * RIPPLE_DISC_R - r) / r : 0.0f;
    if (r > 0.05f) {
      d->rx = d->cx * s;
      d->ry = d->cy * s;
    } else {
      // centred drop: the rim echo returns everywhere at once, just skip it
      d->rx = 1000.0f;
      d->ry = 1000.0f;
    }

    float lambda = ripple_frand(RIPPLE_LAMBDA_MIN, RIPPLE_LAMBDA_MAX);
    d->v = ripple_frand(RIPPLE_SPEED_MIN, RIPPLE_SPEED_MAX);
    d->A = ripple_frand(RIPPLE_AMP_MIN, RIPPLE_AMP_MAX);
    float rings = RIPPLE_RINGS;
    if (random(RIPPLE_BIG_CHANCE) == 0) {
      d->A *= RIPPLE_BIG_AMP;
      lambda *= RIPPLE_BIG_LAMBDA;
      d->v *= RIPPLE_BIG_SPEED;
      rings *= RIPPLE_BIG_RINGS;
    }
    d->inv_lambda = 1.0f / lambda;
    d->lead = 0.5f * lambda;
    d->len = d->lead + rings * lambda;
    d->inv_tail = 1.0f / (d->len - d->lead);
    d->age = 0.0f;
    d->alive = 1;
    return;
  }
}

// signed ring-packet height at squared distance r2 from a source whose front
// is at R. 0 outside the annulus [R - len, R] (culled before the sqrt).
// Envelope: half-cosine rise from the front to the lead crest, then a longer
// half-cosine fall over the tail -> one strong ring with fainter rings behind.
static float ripple_packet(const RippleDrop *p, float r2, float R) {
  if (r2 > R * R) return 0.0f;                           // ahead of the front: still water
  float inner = R - p->len;
  if (inner > 0.0f && r2 < inner * inner) return 0.0f;   // behind the packet
  float d = R - sqrtf(r2);                               // distance behind the front, 0..len
  float u = d - p->lead;
  // crest at the lead, then every wavelength behind it (cos16: 65536 = 2pi)
  float wave = cos16((uint16_t)(int32_t)(u * p->inv_lambda * 65536.0f)) * (1.0f / 32767.0f);
  float e = (u < 0.0f) ? -u / p->lead : u * p->inv_tail; // 0 at the peak, 1 at either end
  float env = 0.5f + 0.5f * (cos16((uint16_t)(e * 32768.0f)) * (1.0f / 32767.0f));
  return wave * env;
}

void ripple() {
  uint32_t now = millis();
  float dt = (now - ripple_last_ms) / 1000.0f;
  if (ripple_last_ms == 0 || dt > 0.1f) dt = 0.03f;   // first frame / after a mode switch
  ripple_last_ms = now;

  // --- spawn / age / retire ---
  ripple_next_drop -= dt;
  if (ripple_next_drop <= 0.0f) {
    ripple_spawn();
    ripple_next_drop = ripple_gap();
  }

  // per-drop amplitude and front radius, computed once per frame
  float amp[RIPPLE_MAX_DROPS];
  float front[RIPPLE_MAX_DROPS];
  float splash[RIPPLE_MAX_DROPS];
  for (int i = 0; i < RIPPLE_MAX_DROPS; i++) {
    RippleDrop *d = &ripple_drops[i];
    amp[i] = 0.0f;
    splash[i] = 0.0f;
    if (!d->alive) continue;
    d->age += dt;
    float R = d->v * d->age;
    front[i] = R;
    amp[i] = d->A * expf(-RIPPLE_DAMP * d->age) / sqrtf(1.0f + R / RIPPLE_SPREAD_R);
    if (d->age < RIPPLE_SPLASH_T) {
      float k = 1.0f - d->age / RIPPLE_SPLASH_T;
      splash[i] = d->A * k * k;
    }
    // done once faint, or once even the rim echo has left the disc
    float reach = 2.0f * RIPPLE_DISC_R + RIPPLE_LAND_R + d->len;
    if (amp[i] < 0.03f || R > reach) {
      d->alive = 0;
      amp[i] = 0.0f;
    }
  }

  ripple_noise_t += (uint16_t)(dt * 400.0f);

  // --- render ---
  for (int l = 0; l < NUM_LEDS; l++) {
    float x = pixel[l].x;
    float y = pixel[l].y;
    float h = 0.0f;

    for (int i = 0; i < RIPPLE_MAX_DROPS; i++) {
      if (amp[i] == 0.0f) continue;
      RippleDrop *d = &ripple_drops[i];
      float dx = x - d->cx, dy = y - d->cy;
      float r2 = dx * dx + dy * dy;
      h += amp[i] * ripple_packet(d, r2, front[i]);
      if (splash[i] > 0.0f)
        h += splash[i] * expf(-r2 / RIPPLE_SPLASH_SIGMA2);
      if (RIPPLE_REFLECT > 0.0f) {
        // rim echo: same front radius from the mirrored source. The rim is a
        // hard wall, so the reflected height keeps its sign (crest -> crest)
        dx = x - d->rx; dy = y - d->ry;
        h += RIPPLE_REFLECT * amp[i] * ripple_packet(d, dx * dx + dy * dy, front[i]);
      }
    }

    // soft-clip so big overlaps glow rather than flatten
    float hc = h / (1.0f + RIPPLE_CLIP * fabsf(h));

    // dim living water underneath
    int base = RIPPLE_BASE +
               ((int)inoise8((pixel[l].hx + 32) * 40, (pixel[l].hy + 32) * 70, ripple_noise_t) - 128) *
               RIPPLE_BASE_WOBBLE / 128;

    float fv = base + (hc > 0.0f ? RIPPLE_GAIN_UP * hc : RIPPLE_GAIN_DOWN * hc);
    if (fv < 0.0f) fv = 0.0f;
    if (fv > 255.0f) fv = 255.0f;
    uint8_t val = (uint8_t)fv;
    if (invert) val = 255 - val;

    float hn = hc > 1.0f ? 1.0f : (hc < -1.0f ? -1.0f : hc);
    uint8_t hu = grad_hue(x, y) + (int8_t)(hn * RIPPLE_HUE_SHIFT);
    uint8_t sat = (uint8_t)(250.0f - (hn > 0.0f ? RIPPLE_GLINT * hn * hn : 0.0f));

    leds[l].setHSV(hu, sat, val);
  }
  FastLED.show();

  delay(RIPPLE_DELAY);
}

#endif
