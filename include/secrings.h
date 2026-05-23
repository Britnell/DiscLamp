#ifndef SECRINGS_H
#define SECRINGS_H

#include "pattern.h"
#include "clock.h"
#include "ring.h"

void secrings_loop() {
  if (millis() - timer >= 1000) {
    timer = millis();
    tick();

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    for (uint8_t b = 0; b < 6; b++) {
      if (s & (0x01 << b))
        draw_ring(b+1);
    }
    FastLED.show();
  }
}

#endif
