#include <Arduino.h>
#include "connect.h"


void setup() {
  wifi_setup();
  // print_ip();
}


void loop() {
  wifi_loop();
  // delay(10);
}