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