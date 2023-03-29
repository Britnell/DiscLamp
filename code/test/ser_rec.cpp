#include <Arduino.h>

void setup() {

  Serial.begin(115200);
  pinMode(D4,OUTPUT);
}

long t = 0;

String ser_buf = "";

void loop() {
  while(Serial.available()>0){
        char c = Serial.read();
        if(c=='\n'){
          if(ser_buf.charAt(0)=='b'){
            digitalWrite(D4,0);            delay(100);
            digitalWrite(D4,1);            delay(100);
          }
          if(ser_buf.charAt(0)=='h'){
            digitalWrite(D4,0);            delay(100);
            digitalWrite(D4,1);            delay(100);
            digitalWrite(D4,0);            delay(100);
            digitalWrite(D4,1);            delay(100);
          }
          if(ser_buf.charAt(0)=='m'){
            digitalWrite(D4,0);   delay(100);
            digitalWrite(D4,1);   delay(100);
            digitalWrite(D4,0);   delay(100);
            digitalWrite(D4,1);   delay(100);
            digitalWrite(D4,0);   delay(100);
            digitalWrite(D4,1);   delay(100);
          }
          
          ser_buf = "";
        }
        else ser_buf += c;
    }

  
}