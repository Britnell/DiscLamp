#ifndef GRADIENT_H
#define GRADIENT_H

#include <math.h>

#define GRAD_MIN_R   1.5f
#define GRAD_MAX_R   5.5f
#define GRAD_TOTAL_R (GRAD_MIN_R + GRAD_MAX_R)

static float grad_angle     = 0.0f;
static float grad_rA        = 3.5f;
static int   grad_ang_n     = 0;
static float grad_ang_v     = 0.0f;
static int   grad_r_n       = 0;
static float grad_r_v       = 0.0f;

void grad_tick() {
  if (grad_ang_n > 0) {
    grad_angle += grad_ang_v;
    grad_ang_n--;
    // Serial.println(grad_angle); 
  } else if (random(10) == 0) {
    grad_ang_n = random(5, 20);
    grad_ang_v = (random(2) ? 1.0f : -1.0f) * (0.02f + random(0, 100) / 100.0f * 0.03f);
    // Serial.print("grad angle impulse  n="); Serial.print(grad_ang_n);
    // Serial.print(" v="); Serial.println(grad_ang_v);
  }

  if (grad_r_n > 0) {
    grad_rA += grad_r_v;
    if (grad_rA > GRAD_MAX_R) grad_rA = GRAD_MAX_R;
    if (grad_rA < GRAD_MIN_R) grad_rA = GRAD_MIN_R;
    grad_r_n--;
  } else if (random(10) == 0) {
    grad_r_n = random(5, 20);
    grad_r_v = (random(2) ? 1.0f : -1.0f) * (0.02f + random(0, 100) / 100.0f * 0.02f);
    Serial.print("grad radius impulse  n="); Serial.print(grad_r_n);
    Serial.print(" v="); Serial.println(grad_r_v);
  }
}

uint8_t grad_hue(float x, float y) {
  float rB = GRAD_TOTAL_R - grad_rA;

  float ax = cosf(grad_angle) * grad_rA;
  float ay = sinf(grad_angle) * grad_rA;
  float bx = -cosf(grad_angle) * rB;
  float by = -sinf(grad_angle) * rB;

  float dA = sqrtf((x-ax)*(x-ax) + (y-ay)*(y-ay));
  float dB = sqrtf((x-bx)*(x-bx) + (y-by)*(y-by));

  float t = dA / (dA + dB);

  return (uint8_t)((int16_t)hue + (int16_t)(t * (int16_t)(hue_a - hue)));
}

#endif
