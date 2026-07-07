#ifndef BINARYCLOCK_H
#define BINARYCLOCK_H

#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"
#include "serial.h"
#include "clock.h"

void binaryclock_2()
{
  uint8_t val, l, b;
  for (l = 0; l < NUM_LEDS; l++)
  {
    leds[l] = CRGB(0, 0, 0);
  }
  // CENTRE <>
  uint8_t wh = 140;
  leds[74].setHSV(0, 128, wh);
  leds[76].setHSV(0, 128, wh);
  leds[87].setHSV(0, 128, wh);
  leds[88].setHSV(0, 128, wh);
  leds[62].setHSV(0, 128, wh);
  leds[63].setHSV(0, 128, wh);
  leds[50].setHSV(0, 128, wh);
  leds[100].setHSV(0, 128, wh);

  for (b = 0; b < 6; b++)
  {
    val = (s & (0x01 << b)) ? 255 : 0;
    leds[38 + b].setHSV(hue_a, 250, val);
    leds[37 - b].setHSV(hue_a, 250, val);
    leds[113 + b].setHSV(hue_a, 250, val);
    leds[112 - b].setHSV(hue_a, 250, val);

    // val = (m & (0x01 << b )) ? 255 : 0;
    leds[50 + 1 + b].setHSV(hue_b, 250, val);
    leds[50 - 1 - b].setHSV(hue_b, 250, val);
    leds[100 + 1 + b].setHSV(hue_b, 250, val);
    leds[100 - 1 - b].setHSV(hue_b, 250, val);

    if (b < 4)
    {
      // val = (h & (0x01 << b )) ? 255 : 0;
      leds[75 + 2 + b].setHSV(hue, 250, val);
      leds[75 - 2 - b].setHSV(hue, 250, val);
      leds[89 + b].setHSV(hue, 250, val);
      leds[86 - b].setHSV(hue, 250, val);
      leds[64 + b].setHSV(hue, 250, val);
      leds[61 - b].setHSV(hue, 250, val);
    }
  }
  FastLED.show();
}

void binaryclock_loop()
{
  if (millis() - timer >= 1000)
  {
    timer = millis();
    tick();
    binaryclock_2();
  }
}

#endif

// void binaryclock_1()
// {
//   uint8_t val, l, b;
//   for (l = 0; l < NUM_LEDS; l++)
//   {
//     leds[l] = CRGB(0, 0, 0);
//   }
//   for (b = 0; b < 6; b++)
//   {
//     val = (s & (0x01 << b)) ? 255 : 0;
//     leds[75 - b].setHSV(hue, 250, val);
//     leds[75 + b].setHSV(hue, 250, val);

//     val = (m & (0x01 << b)) ? 255 : 0;
//     leds[100 - b].setHSV(hue, 250, val);
//     leds[100 + b].setHSV(hue, 250, val);
//     leds[50 - b].setHSV(hue, 250, val);
//     leds[50 + b].setHSV(hue, 250, val);

//     if (b < 4)
//     {
//       val = (h & (0x01 << b)) ? 255 : 0;
//       leds[27 - b].setHSV(hue, 250, val);
//       leds[27 + b].setHSV(hue, 250, val);
//       leds[124 - b].setHSV(hue, 250, val);
//       leds[124 + b].setHSV(hue, 250, val);
//     }
//   }
//   FastLED.show();
// }

// void binaryclock_3()
// {
//   uint8_t val, l, b;
//   for (l = 0; l < NUM_LEDS; l++)
//   {
//     leds[l] = CRGB(0, 0, 0);
//   }
//   for (b = 0; b < 6; b++)
//   {
//     val = (s & (0x01 << b)) ? 255 : 0;
//     leds[26 + b].setHSV(hue_a, 250, val);
//     leds[25 - b].setHSV(hue_a, 250, val);

//     // val = (m & (0x01 << b )) ? 255 : 0;
//     leds[50 - 1 - b].setHSV(hue, 250, val);
//     leds[50 + 1 + b].setHSV(hue, 250, val);
//     leds[37 - b].setHSV(hue, 250, val);
//     leds[38 + b].setHSV(hue, 250, val);
//     leds[100 + 1 + b].setHSV(hue, 250, val);
//     leds[100 - 1 - b].setHSV(hue, 250, val);
//     leds[113 + b].setHSV(hue, 250, val);
//     leds[112 - b].setHSV(hue, 250, val);

//     if (b < 4)
//     {
//       // val = (h & (0x01 << b )) ? 255 : 0;
//       leds[75 + 2 + b].setHSV(hue_b, 250, val);
//       leds[75 - 2 - b].setHSV(hue_b, 250, val);
//       leds[88 + 1 + b].setHSV(hue_b, 250, val);
//       leds[87 - 1 - b].setHSV(hue_b, 250, val);
//       leds[63 + 1 + b].setHSV(hue_b, 250, val);
//       leds[62 - 1 - b].setHSV(hue_b, 250, val);
//     }
//   }
//   FastLED.show();
// }