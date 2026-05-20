#include <Arduino.h>

void setup() {

  Serial.begin(115200);

  Serial.println(" ready ");
}

long t = 0;
void loop() {
  while(Serial.available()){
    char c=  Serial.read();
    Serial.print(c);
  }
  
  // else if(mode.equals("cross"))   cross();
  // else if(mode.equals("count"))    //   count();
  // else if(mode.equals("fast"))   fast();
  
  if(millis()-t>2000){
    t = millis();
    Serial.println(" IZ ME!");
  }

}