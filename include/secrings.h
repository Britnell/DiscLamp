#ifndef SECRINGS_H
#define SECRINGS_H

#include "pixel.h"
#include "pattern.h"
#include "clock.h"
#include "ring.h"

void secrings_loop() {
  if (millis() - timer >= 1000) {
    timer = millis();
    tick();
    
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    
    leds[4].setHSV(hue_a, 250, 127);
    leds[10].setHSV(hue_a, 250, 127);
    leds[69].setHSV(hue_a, 250, 127);
    leds[81].setHSV(hue_a, 250, 127);
    leds[140].setHSV(hue_a, 250, 127);
    leds[146].setHSV(hue_a, 250, 127);

    for (uint8_t b = 0; b < 6; b++) {
      if (s & (0x01 << b))
        draw_ring(b+1);
    }
    FastLED.show();
  }
}

#endif
