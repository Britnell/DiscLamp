#ifndef PATTERN_H
#define PATTERN_H

#include "pixel.h"
#include "lib.h"

String mode = "clock";
unsigned short bright = 100;
unsigned short hue = 0;
short invert = 0;
float f = 0;
long timer = 0;



int find_pixel(float x, float y){
  for(uint8_t l=0;l<NUM_LEDS;l++){
    int d = abs(x - pixel[l].x ) + abs(y-pixel[l].y);
    if(d<0.3) {
      return l;
    }
  }
  return -1;
}


// * * *   

void full(){
  for(int l=0;l<NUM_LEDS;l++){
      leds[l].setHSV(hue,250, 255);
  }
  FastLED.show();    
  delay(60);
} 


void half(){
  int val = 0;
  for(int l=0;l<NUM_LEDS;l++){
    if(invert){
      if(pixel[l].x<=0)      val = 255;
      else                  val = 0;
    }
    else{
      if(pixel[l].x>=0)      val = 255;
      else                  val = 0;
    }
    leds[l].setHSV(hue,250, val);
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
        val = 255;

    if(invert){
      if(val==0)  val = 255;
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
      val = 255;
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
      val = 255;
    else if(p.y< 1.8 && p.y> -1.8)
      val = 255;
    
    if(invert){
      if(val==0)  val = 255;
      else        val = 0;
    }
    leds[l].setHSV( hue ,250,  val );
  }
  FastLED.show();  

  delay(100);
}


  float ratio = 0.33;
  float off = 0;
  
void circle(){
  int lim = 6;
  int val;
  for(int l=0;l<NUM_LEDS;l++){
    float r = sqrt( pixel[l].x*pixel[l].x + pixel[l].y*pixel[l].y );
    if(r>5) val = 255;
    else val = 0;

    float col = cos(ratio * PI) * pixel[l].y - pixel[l].x * sin(ratio *PI) + off * lim ;
    if(col<-lim)col=-lim; else if(col>lim)col=lim;

    int h = int(linear(col, -lim,lim, hue, hue+20 ));
    // p(l);p(" , ");p(c); p(" , "); pl(val);

    leds[l].setHSV( h ,250,  val );
  }
  FastLED.show();

  ratio += 0.1;
  // if(ratio>1.0) ratio = 0;
  // pl(ratio);
  delay(100);
}

void hatch(){
  for(int l=0;l<NUM_LEDS;l++){
    
    int y = round(pixel[l].y / DY ) + 10*3;
    int x = round( pixel[l].x );
    int h = round( pixel[l].x + 0.5) + 10*3;
    
    int val = 0;
    if(y%6==0){
      if(x%3==0)    val = 255;
    }
    if(y%6==1 || y%6==5){
      if(h%3==0)    val = 255;
      if(h%3==1)    val = 255;
    }
    else if(y%6==2 || y%6==4){
      if(x%3!=0)  val = 255;
    }
    else if(y%6==3){
      if(h%3==2)  val = 255;
    }
    
    if(invert){
      if(val==0) val = 255;
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
      if(x%4==0)                val = 255;
    }
    else if(y%8==1 || y%8==7){
      if(h%4==0 || h%4==1)      val = 255;
    }
    else if(y%8==2|| y%8==6){
      if(x%4==1 || x%4==3)      val = 255;
    }
    else if(y%8==3|| y%8==5){
      if(h%4==2 || h%4==3)      val = 255;
    }
    else if(y%8==4){
      if(x%4==2)                val = 255;
    }

    if(invert){
      if(val==0) val = 255;
      else       val = 0;
    }
    leds[l].setHSV( hue ,250, val );  
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
      if((x-row/2+10*Q)%Q==0)    val = 255;
    }
    else{
      x = abs(int(pixel[l].x));
      if((x-row/2+10*Q)%Q==0)    val = 255;
    }

    if(invert){
      if(val==0) val = 255;  else  val = 0;
    }
    leds[l].setHSV( hue ,255,  val );
  }
  FastLED.show();  

  delay(100);
}


uint8_t triforce[45] = {
  33,34,35,36,37,38,39,40,41,42,
  46, 47, 48, 49,   51,52,53,54,
  59,60,61,         64,65,66,
  72,73,            77,78,
  85,               90,
  98, 99, 100, 101, 102,
  111, 112, 113, 114,
  123,124,125,
  134,135,
  143
};

void trif(){
  int val;
  for(int l=0;l<NUM_LEDS;l++){
    val = 0;
    for(int t=0;t<45;t++){
      if(triforce[t]==l){
        val = 255;
        break;
      }
      if(triforce[t]>l)
        break;
    }
    leds[l].setHSV( hue ,255,  val );
  }
  FastLED.show();
  delay(100);
}


// * * * * * * * * * * * *  * * * * * * * * *  ** * * * * * * * * * * * 

float f_cap(float p){
  if(p<0) p = -p;
  while(p>1.0){   p -= 1.0;   }
  return p;
}

void waves(){
  float q = 6.0;    // every Q repeat
  float r = 0.3;    // 0.3*6 = true rad
  
  for(int l=0;l<NUM_LEDS;l++){
    int val;
    float p = 1.0 * (pixel[l].row_i +f ) / q ;
    p = f_cap(p);
    
    if(!invert){
      val = 0;
      if(p>1-r)
        val = int( linear(p, 1-r,1, 0,255)  );
      else if(p<r)
        val = int( linear(p, 0,r, 255,0)  );
    }
    else {
      val = 255;
      if(p>1-r)
        val = int( linear(p, 1-r,1, 255,0)  );
      else if(p<r)
        val = int( linear(p, 0,r, 0,255)  );
    }
    
    leds[l].setHSV( hue ,250,  val );

  }
  FastLED.show();  

  if(every(5)){
    f += 0.1;
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
        val = 255;
      else if(d<w+r)
        val = int( linear(d, w,w+r, 255,0) );
      else
        val = 0;
    }
    else {
      if(d<w)
        val = 0;
      else if(d<w+r)
        val = int( linear(d, w,w+r, 0,255) );
      else
        val = 255;
    }      
    leds[l].setHSV( hue,250, val );  
  }
  FastLED.show();  
  
  delay(100);
} 



void lines(){
  int D = 3;
  
  for(int l=0;l<NUM_LEDS;l++){
    int x = round(pixel[l].x + pixel[l].y/DY *0.5 ) ;
    int val;

    if(!invert){
      val = 0;
      if(x%D==0)      val = 255;
    }
    else {
      val = 255;
      if(x%D==0)      val = 0;
    }
    
    leds[l].setHSV( hue ,250, val );  
  }
  FastLED.show();  
  
  delay(100);
} 



void stripes(){
  int val;
  float tmp;

  for(int l=0;l<NUM_LEDS;l++){
    tmp = sin( (pixel[l].x + pixel[l].y) /1 );
    val = int((tmp + 1) * 255);

    leds[l].setHSV( hue ,250,  val );
  }
  FastLED.show();  

  delay(100);
}


void scroll_lines(){
  float q = 6.0;    // every Q repeat
  float r = 0.3;    // 0.3*6 = true rad
  
  int val;
  
  for(int l=0;l<NUM_LEDS;l++){
    float p = 1.0 * (pixel[l].x + pixel[l].y / DY * 0.5 +f ) / q ;
    p = f_cap(p);
    
    if(!invert){
      val = 0;
      if(p>1-r)
        val = int( linear(p, 1-r,1, 0,255 )  );
      else if(p<r)
        val = int( linear(p, 0,r, 255,0)  );
    }
    else{
      val = 255;
      if(p>1-r)
        val = int( linear(p, 1-r,1, 255,0 )  );
      else if(p<r)
        val = int( linear(p, 0,r, 0,255)  );
    }
    
    leds[l].setHSV( hue ,250,  val );
  }
  FastLED.show();  

  f += 0.01;
  // if(every(5)){
  //   if(++d>=255)  d=0;
  // }

  delay(20);
} 





#endif

