#ifndef PIXEL_PATTERNS
#define PIXEL_PATTERNS

#include "pixel.h"

//std::function<bool (int)> callback

// int   d = 0;  // hue ?
float f = 0;

void full(){
  for(int l=0;l<NUM_LEDS;l++){
      leds[l].setHSV(hue,250, bright);
  }
  FastLED.show();  
  
  // d++;
  // if(d>=255) d = 0;
  
  delay(60);
} 


void half(){
  int val = 0;
  for(int l=0;l<NUM_LEDS;l++){
    if(invert){
      if(pixel[l].x<=0)      val = bright;
      else                  val = 0;
    }
    else{
      if(pixel[l].x>=0)      val = bright;
      else                  val = 0;
    }
    leds[l].setHSV(hue,250, val);
  }
  FastLED.show();  
  delay(100);
} 


void perc_a(){
  // f 
  float q = 6.0;    // every Q repeat
  float r = 0.3;    // 0.3*6 = true rad
  
  
  for(int l=0;l<NUM_LEDS;l++){
    int val;
    float p = 1.0 * (pixel[l].row_i +f ) / q ;
    p = f_cap(p);
    
    if(!invert){
      val = 0;
      if(p>1-r)
        val = int( linear(p, 1-r,1, 0,bright)  );
      else if(p<r)
        val = int( linear(p, 0,r, bright,0)  );
    }
    else {
      val = bright;
      if(p>1-r)
        val = int( linear(p, 1-r,1, bright,0)  );
      else if(p<r)
        val = int( linear(p, 0,r, 0,bright)  );
    }
    
    leds[l].setHSV( hue ,250,  val );

  }
  FastLED.show();  

  if(every(5)){
    f += 0.1;
//    if(f>q)f = 0;
  }
  
  delay(20);
} 


void line(){
  float w = 1.8;
  float r = 2.2;
  float m = 0.86;
  
  for(int l=0;l<NUM_LEDS;l++){
    float d = pixel[l].x - m * pixel[l].y;
    if(d<0) d = -d;
    int val;

    if(!invert){
      if(d<w)
        val = bright;
      else if(d<w+r)
        val = int( linear(d, w,w+r, bright,0) );
      else
        val = 0;
    }
    else {
      if(d<w)
        val = 0;
      else if(d<w+r)
        val = int( linear(d, w,w+r, 0,bright) );
      else
        val = bright;
    }      
    leds[l].setHSV( hue,250, val );  
  }
  FastLED.show();  
  
  // d++;
  // if(d>=255) d = 0;
  
  delay(100);
} 



void lines(){

  int ON = 200;
  int D = 3;
  
  for(int l=0;l<NUM_LEDS;l++){
    int x = round(pixel[l].x + pixel[l].y/DY *0.5 ) ;
    int val;

    if(!invert){
      val = 0;
      if(x%D==0)      val = bright;
    }
    else {
      val = bright;
      if(x%D==0)      val = 0;
    }
    
    leds[l].setHSV( hue ,250, val );  
  }
  FastLED.show();  
  
  // d++;
  // if(d>=255) d = 0;
  
  delay(100);
} 

void hatch(){
  
  for(int l=0;l<NUM_LEDS;l++){
    
    int y = round(pixel[l].y / DY ) + 10*3;
    int x = round( pixel[l].x );
    int h = round( pixel[l].x + 0.5) + 10*3;
    
    int val = 0;
    if(y%6==0){
      if(x%3==0)    val = bright;
    }
    if(y%6==1 || y%6==5){
      if(h%3==0)    val = bright;
      if(h%3==1)    val = bright;
    }
    else if(y%6==2 || y%6==4){
      if(x%3!=0)  val = bright;
    }
    else if(y%6==3){
      if(h%3==2)  val = bright;
    }
    
    if(invert){
      if(val==0) val = bright;
      else       val = 0;
    }
    
    leds[l].setHSV( hue ,250, val );  
  }
  FastLED.show();  
  
  delay(100);
} 


void hatch2(){
  
  for(int l=0;l<NUM_LEDS;l++){
    
    int y = round(pixel[l].y / DY ) + 10*3  +6;  // +2 +6
    int x = round( pixel[l].x ) + 10*3;
    int h = round( pixel[l].x + 0.5) + 10*3;
    
    int val = 0;
    if(y%8==0){
      if(x%4==0)                val = bright;
    }
    else if(y%8==1 || y%8==7){
      if(h%4==0 || h%4==1)      val = bright;
    }
    else if(y%8==2|| y%8==6){
      if(x%4==1 || x%4==3)      val = bright;
    }
    else if(y%8==3|| y%8==5){
      if(h%4==2 || h%4==3)      val = bright;
    }
    else if(y%8==4){
      if(x%4==2)                val = bright;
    }

    if(invert){
      if(val==0) val = bright;
      else       val = 0;
    }
    leds[l].setHSV( hue ,250, val );  
  }
  FastLED.show();  
  
  delay(100);
} 


// 2nd hatch - larger


void scroll_lines(){
  // f 
  float q = 6.0;    // every Q repeat
  float r = 0.3;    // 0.3*6 = true rad
  
  int val;
  
  for(int l=0;l<NUM_LEDS;l++){
    float p = 1.0 * (pixel[l].x + pixel[l].y / DY * 0.5 +f ) / q ;
    p = f_cap(p);
    
    if(!invert){
      val = 0;
      if(p>1-r)
        val = int( linear(p, 1-r,1, 0,bright )  );
      else if(p<r)
        val = int( linear(p, 0,r, bright,0)  );
    }
    else{
      val = bright;
      if(p>1-r)
        val = int( linear(p, 1-r,1, bright,0 )  );
      else if(p<r)
        val = int( linear(p, 0,r, 0,bright)  );
    }
    
    leds[l].setHSV( hue ,250,  val );
  }
  FastLED.show();  

  f += 0.01;
  if(every(5)){
    // if(++d>=255)  d=0;
  }

  delay(20);
} 



void stripes(){
  int val;
  float tmp;

  for(int l=0;l<NUM_LEDS;l++){
    tmp = sin( (pixel[l].x + pixel[l].y) /1 );
    val = int((tmp + 1) * 127);

    leds[l].setHSV( hue ,250,  val );
  }
  FastLED.show();  

  delay(100);
}



void square(){
  int val;
  float R;
  LED_STRUCT p;

  for(int l=0;l<NUM_LEDS;l++){
    p = pixel[l];
    val = 0;

    if(!invert)  R = 5;
    else         R = 4.2; // 4.2 , 4.4 
    
    if( p.x<R && p.x> -R )
      if(p.y<R && p.y> -R )
        val = bright;

    if(invert){
      if(val==0)  val = bright;
      else        val = 0;
    }

    leds[l].setHSV( hue ,250,  val );
  }
  FastLED.show();  

  delay(100);
}

void triangle(){
  int val;
  float L, R;
  for(int l=0;l<NUM_LEDS;l++){
    L = pixel[l].y + 1.6 * pixel[l].x + 7 ;
    R = pixel[l].y - 1.6 * pixel[l].x + 7 ;
    
    if( L>0 && R>0 && pixel[l].y < 4 )       
      val = bright;
    else            val = 0;
    
    leds[l].setHSV( hue ,250,  val );
  }
  FastLED.show();  

  delay(100);
}


void cross(){
  int val;
  LED_STRUCT p;

  for(int l=0;l<NUM_LEDS;l++){
    p = pixel[l];
    val = 0;
    if(p.x< 2.1 && p.x> -2.1 )
      val = bright;
    else if(p.y< 1.8 && p.y> -1.8)
      val = bright;
    
    if(invert){
      if(val==0)  val = bright;
      else        val = 0;
    }
    leds[l].setHSV( hue ,250,  val );
  }
  FastLED.show();  

  delay(100);
}


void arrow(){
  int Q = 3;
  int val,x,row;

  for(int l=0;l<NUM_LEDS;l++){
    val = 0;
    row = abs( int(pixel[l].y/DY) );

    if(row%2==0){
      x = abs( int(pixel[l].x) );
      if((x-row/2+10*Q)%Q==0)    val = bright;
    }
    else{
      x = abs(int(pixel[l].x));
      if((x-row/2+10*Q)%Q==0)    val = bright;
    }

    if(invert){
      if(val==0) val = bright;  else  val = 0;
    }
    leds[l].setHSV( hue ,255,  val );
  }
  FastLED.show();  

  delay(100);
}

// * RAIN

uint8_t rain_initialised = 0;
#define NUM_PART  8

float rain_rad = 1;
float rain_speed = 0.04;

typedef struct {
  float x;
  float y;
} PARTICLE;


PARTICLE particles[NUM_PART];

void rain(){
  PARTICLE p;

  if(!rain_initialised){
    for(int r=0;r<NUM_PART;r++){
      p.x = linear(random(0,255),0,255,-6,6);
      p.y = linear(r,0,NUM_PART+1,-6,6);
      // p.y = linear(random(0,255),0,255,-6,6);
      particles[r] = p;
    }
    rain_initialised = 1;
  }

  int val;
  float x, d, dist;
  
  // * pixels
  for(int l=0;l<NUM_LEDS;l++){
    dist = 10;
    for(int r=0;r<NUM_PART;r++){
      x = pixel[l].x-particles[r].x;
      d = x*x;
      x = pixel[l].y -particles[r].y;
      d += x*x;
      if(d<dist)  dist = d;
    }

    val = 0;
    if(dist<rain_rad)
      val = int(expon( (rain_rad-dist), 0,rain_rad, 0,bright) );

    leds[l].setHSV( hue ,250,  val );
  }
  FastLED.show();  

  // * step
  for(int r=0;r<NUM_PART;r++){
    particles[r].y += rain_speed;

    if(particles[r].y>6) {
      particles[r].x = linear(random(0,255),0,255,-6,6);
      particles[r].y = -6;
      // particles[r].y = linear(random(0,255),0,255,-6,-8);
    }
  }

  delay(100);
}



void empty(){
  int val;
  for(int l=0;l<NUM_LEDS;l++){
    val = l;
    leds[l].setHSV( hue ,250,  val );
  }
  FastLED.show();  

  delay(100);
}



#endif
