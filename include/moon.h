#pragma once
#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"
#include <math.h>

// Sphere radius slightly outside the disc so all pixels have a real pz
#define MOON_R 7.0f

// 0 = new moon, PI = full moon, 2*PI = new moon again
float moon_phase_angle = 0.0f;

void moon() {
  float R = MOON_R;

  for(int l = 0; l < NUM_LEDS; l++) {
    float px = pixel[l].x;
    float py = pixel[l].y;
    float r2 = px*px + py*py;

    // z-component of the surface normal on the sphere
    float pz = sqrtf(R*R - r2);

    // Sun direction: (sin(phase), 0, -cos(phase))
    //   phase=0   -> sun at (0,0,-1): behind the moon -> new moon (all dark)
    //   phase=PI  -> sun at (0,0,+1): facing observer -> full moon (lit, limb-darkened)
    //   phase=PI/2 -> sun at (+1,0,0): right quarter
    float raw = px * sinf(moon_phase_angle) - pz * cosf(moon_phase_angle);
    if(raw < 0.0f) raw = 0.0f;
    float intensity = raw / R;   // 0..1

    leds[l].setHSV(grad_hue(px, py), 250, (uint8_t)(intensity * 255.0f));
  }

  FastLED.show();

  moon_phase_angle += 0.008f;
  if(moon_phase_angle >= 2.0f * M_PI) moon_phase_angle -= 2.0f * M_PI;

  delay(30);
}
