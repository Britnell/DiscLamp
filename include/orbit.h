#ifndef ORBIT_H
#define ORBIT_H

#include "connect.h"

int orbit_speed = 200;

typedef struct {
  float x;
  float y;
  float vx;
  float vy;
  float ax;
  float ay;
  float rad;
} Orbit;



float random_angle(){
    return linear( random(0,100), 0,100, 0, 2*PI );
}


int orbit_f = 0;
void orbit_impulse(Orbit *orb){
    int every_s = 8 * 50;
    if( orbit_f++ % every_s == 0){
        if(random(4)==0){
            float ang = random_angle();
            orb->ax = cos(ang);
            orb->ay = sin(ang);
        }
        
    }
}

void step_orbit(Orbit *orb){
    float max_sp = 2;
    
    // apply
    orb->vx += orb->ax / orbit_speed;
    orb->vy += orb->ay / orbit_speed;
    orb->x += orb->vx / orbit_speed;
    orb->y += orb->vy / orbit_speed;

    // stop when reach velocity of 1 in any dir
    if(orb->vx>max_sp){
        orb->vx = max_sp;
        orb->ax = 0;    orb->ay = 0;
    }
    if(orb->vx<-max_sp){
        orb->vx = -max_sp;
        orb->ax = 0;    orb->ay = 0;
    }
    if(orb->vy>max_sp){
        orb->vy = max_sp;
        orb->ax = 0;    orb->ay = 0;
    }
    if(orb->vy<-max_sp){
        orb->vy = -max_sp;
        orb->ax = 0;    orb->ay = 0;
    }

    // bounce off edges
    float dist = sqrt(orb->x * orb->x + orb->y * orb->y);
    if(dist> 5){
        float mag = sqrt(orb->vx * orb->vx + orb->vy * orb->vy);
        orb->ax = 0;
        orb->ay = 0;

        // turn around 180
        float ang = atan2(orb->y,orb->x);
        ang += PI + linear(random(0,100), 0,100, -0.8, 0.8 );
        orb->vx = mag * cos(ang);
        orb->vy = mag * sin(ang);
        step_orbit(orb);
    }

    // random impulses
    orbit_impulse(orb);

}


// void print_orbit(Orbit* orb){
//     p(" O  ,  p: "); 
//     p(orb->x); p(" , "); p(orb->y); p(" \t v: ");
//     p(orb->vx); p(" , "); p(orb->vy); p(" \t a: ");
//     p(orb->ax); p(" , "); p(orb->ay); p(" , ");
//     pl();
// }


#endif