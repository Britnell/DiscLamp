#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "secrets.h"

// LEDS -----

#include <FastLED.h>

#define NUM_LEDS   150
#define DATA_PIN   D4
// #define CLOCK_PIN  0

CRGB leds[NUM_LEDS];


const char* ssid = STASSID;
const char* password = STAPSK;

void setup() {
  
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  leds[74].setRGB(0,0,20);
  leds[75].setRGB(0,0,250);
  leds[76].setRGB(0,0,20);
  FastLED.show();

}

int x = 0;

void loop() {
  
  for(int l=0;l<NUM_LEDS;l++){
    if(l>=x && l<x+4)
      leds[l].setRGB(0,80,20);
    else
      leds[l] = 0x000000;
  }
  leds[0] = 0x000000;
  FastLED.show();
  if(++x>=NUM_LEDS) x = 0;
  
  delay(10);
}