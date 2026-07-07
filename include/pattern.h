#ifndef PATTERN_H
#define PATTERN_H

#include "pixel.h"
#include "lib.h"

String mode = "scroll_lines";

#define HUE_DIFF 10
unsigned short bright = 50;
unsigned short hue = 1;
unsigned short hue_a = hue + HUE_DIFF;
unsigned short hue_b = (hue + 256 - HUE_DIFF) % 256;
short invert = 0;
uint8_t param = 50;
float f = 0;
long timer = 0;

#include "gradient.h"



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
      leds[l].setHSV(grad_hue(pixel[l].x, pixel[l].y),250, 255);
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
    leds[l].setHSV(grad_hue(pixel[l].x, pixel[l].y),250, val);
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

    leds[l].setHSV( grad_hue(pixel[l].x, pixel[l].y) ,250,  val );
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
    
    leds[l].setHSV( grad_hue(pixel[l].x, pixel[l].y) ,250,  val );
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
    leds[l].setHSV( grad_hue(pixel[l].x, pixel[l].y) ,250,  val );
  }
  FastLED.show();  

  delay(100);
}



void circle(){
  static float ratio = 0.33;
  static float breathPhase = 0;
  int lim = 6;
  float breatheAmp = 1.5;
  float baseRadius = 3.5;
  float breathe = baseRadius + breatheAmp * sin(breathPhase * 2 * PI);

  for(int l=0;l<NUM_LEDS;l++){
    float r = sqrt( pixel[l].x*pixel[l].x + pixel[l].y*pixel[l].y );

    float outerEdge = 6.5;
    float brightness;
    if(!invert)
      brightness = constrain((r - breathe) / (outerEdge - breathe), 0.0, 1.0);
    else
      brightness = constrain((breathe - r) / breathe, 0.0, 1.0);
    int val = int(brightness * 255);

    float col = cos(ratio * PI) * pixel[l].y - pixel[l].x * sin(ratio * PI);
    if(col<-lim)col=-lim; else if(col>lim)col=lim;

    int h = int(linear(col, -lim,lim, hue, hue+20 ));

    leds[l].setHSV( h, 250, val );
  }
  FastLED.show();

  ratio += 0.1;
  breathPhase += 1.0 / 60.0;
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
    
    leds[l].setHSV( grad_hue(pixel[l].x, pixel[l].y) ,250,  val );

  }
  FastLED.show();  

  if(every(5)){
    f += 0.1;
  }
  
  delay(20);
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
    
    leds[l].setHSV( grad_hue(pixel[l].x, pixel[l].y) ,250, val );  
  }
  FastLED.show();  
  
  delay(100);
} 



static float sl_angle      = 0.0f;
static float sl_width      = 0.3f;
static int   sl_frames_left = 0;
static float sl_angle_vel  = 0.0f;
static float sl_width_vel  = 0.0f;

#define SL_MAX_ANGLE 0.785f
#define SL_MIN_WIDTH 0.3f
#define SL_MAX_WIDTH 0.167f
#define SL_SCROLL_RANGE 6.0f
#define SL_SCROLL_SPEED 0.03f

void scroll_lines_tick() {
  if (sl_frames_left > 0) {
    sl_angle += sl_angle_vel;
    if (sl_angle >  SL_MAX_ANGLE) sl_angle =  SL_MAX_ANGLE;
    if (sl_angle < -SL_MAX_ANGLE) sl_angle = -SL_MAX_ANGLE;
    sl_width += sl_width_vel;
    if (sl_width > SL_MAX_WIDTH) sl_width = SL_MAX_WIDTH;
    if (sl_width < SL_MIN_WIDTH) sl_width = SL_MIN_WIDTH;
    sl_frames_left--;
  } else if (random(10) == 0) {
    sl_frames_left = random(120, 240);
    sl_angle_vel   = (random(2) ? 1.0f : -1.0f) * random(1, 3) * 0.001f;
    sl_width_vel   = (random(2) ? 1.0f : -1.0f) * random(2, 6) * 0.001f;
  }
}

void scroll_lines(){
  scroll_lines_tick();

  for(int l=0;l<NUM_LEDS;l++){
    float axis_pos = cosf(sl_angle) * pixel[l].x + sinf(sl_angle) * pixel[l].y;
    float pos = f_cap((axis_pos + f) / SL_SCROLL_RANGE);
    int val;

    if(!invert){
      val = 0;
      if(pos > 1-sl_width)
        val = int( linear(pos, 1-sl_width, 1, 0, 255) );
      else if(pos < sl_width)
        val = int( linear(pos, 0, sl_width, 255, 0) );
    }
    else{
      val = 255;
      if(pos > 1-sl_width)
        val = int( linear(pos, 1-sl_width, 1, 255, 0) );
      else if(pos < sl_width)
        val = int( linear(pos, 0, sl_width, 0, 255) );
    }

    leds[l].setHSV( grad_hue(pixel[l].x, pixel[l].y), 250, val );
  }
  FastLED.show();

  f += SL_SCROLL_SPEED;
  delay(20);
}





#endif

