#ifndef PIXEL_FUNC
#define PIXEL_FUNC
#include <Arduino.h>

#define p(x) Serial.print(x)
#define pl(x) Serial.println(x)

float expon(float x, float xmin, float xmax, float omin, float omax ){
  float prop = (x-xmin) / (xmax-xmin);
  return omin + (omax-omin) * prop * prop ;
}

float linear(float x, float xmin, float xmax, float omin, float omax ){
  float prop = (x-xmin) / (xmax-xmin);
  return omin + (omax-omin) * prop ;
}


// Hue difference scaling: smaller near reds (0/255), larger in blues (~128)
uint8_t hue_diff(uint8_t h) {
    if (h < 128)
        return (uint8_t)linear(h, 0, 128, 10, 26);
    else
        return (uint8_t)linear(h, 128, 255, 26, 10);
}

int e = 0;

int every(int num){
  e++;
  if(e>=num){
    e = 0;
    return 1;
  }
  else  return 0;
}



#endif