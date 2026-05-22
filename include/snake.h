#pragma once
#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

#define SNAKE_MOVE_FRAMES  50
#define SNAKE_TURN_PROB    30   // % chance to pick a new random direction each step
#define SNAKE_VARY_PROB    10   // % chance to nudge target length up or down each step
#define SNAKE_MAX_COUNT     4   // maximum number of simultaneous snakes
#define SNAKE_MIN_LEN       5
#define SNAKE_MAX_LEN      14

// Mirror modes — every painted pixel is also drawn at its mirrored position
#define MIRROR_NONE 0
#define MIRROR_X    1   // flip horizontally (x -> -x)
#define MIRROR_Y    2   // flip vertically   (y -> -y)
#define MIRROR_XY   3   // flip both axes    (x -> -x, y -> -y)
uint8_t snake_mirror = MIRROR_X;
uint8_t num_snakes   = 2;

typedef struct {
    uint8_t body[SNAKE_MAX_LEN];  // [0]=head, [len-1]=tail; pixel indices
    uint8_t len;
    uint8_t target_len; // length the snake is trying to reach; len grows/shrinks toward this
    uint8_t dir;        // 0=left 1=right 2=up-R 3=up-L 4=dn-R 5=dn-L
    uint8_t frame;      // counts 0..SNAKE_MOVE_FRAMES between steps
    uint8_t next_head;  // pixel fading in this cycle
    uint8_t old_tail;   // pixel fading out this cycle (only valid if dropped_tail)
    bool    dropped_tail; // did this step drop a tail? (false while growing)
} SNAKE;

SNAKE snakes[SNAKE_MAX_COUNT];


// --- mirror helpers ---

int snake_mirror_of(uint8_t idx) {
    if(snake_mirror == MIRROR_NONE) return -1;
    LED_STRUCT p = pixel[idx];
    float mx = (snake_mirror == MIRROR_X || snake_mirror == MIRROR_XY) ? -p.x : p.x;
    float my = (snake_mirror == MIRROR_Y || snake_mirror == MIRROR_XY) ? -p.y : p.y;
    return find_pixel(mx, my);
}

void snake_paint(uint8_t idx, uint8_t h, uint8_t v) {
    leds[idx].setHSV(h, 250, v);
    int m = snake_mirror_of(idx);
    if(m >= 0 && m != idx) leds[m].setHSV(h, 250, v);
}

// --- next-field lookup with edge wrapping ---

int snake_next_field(uint8_t idx, uint8_t dir) {
    LED_STRUCT pix = pixel[idx];
    int n = -1;
    switch(dir) {
        case 0: if(idx > 0           && pixel[idx-1].row == pix.row) n = idx-1; break;
        case 1: if(idx < NUM_LEDS-1  && pixel[idx+1].row == pix.row) n = idx+1; break;
        case 2: n = find_pixel(pix.x+0.5f, pix.y+DY);  break;
        case 3: n = find_pixel(pix.x-0.5f, pix.y+DY);  break;
        case 4: n = find_pixel(pix.x+0.5f, pix.y-DY);  break;
        case 5: n = find_pixel(pix.x-0.5f, pix.y-DY);  break;
    }
    if(n != -1) return n;

    // wrap: left/right stay in same row, diagonals jump to antipodal pixel
    if(dir == 0) { for(int l=NUM_LEDS-1; l>=0; l--) if(pixel[l].row==pix.row) return l; }
    if(dir == 1) { for(int l=0; l<NUM_LEDS; l++)    if(pixel[l].row==pix.row) return l; }
    float wx = -pix.x, wy = -pix.y;
    int best = 0; float best_d = 9999.0f;
    for(int l=0; l<NUM_LEDS; l++) {
        float dx = pixel[l].x - wx, dy = pixel[l].y - wy;
        float d  = dx*dx + dy*dy;
        if(d < best_d) { best_d = d; best = l; }
    }
    return best;
}


// --- state machine ---

void snake_init(SNAKE &s, uint8_t start) {
    s.len          = 1;
    s.target_len   = SNAKE_MIN_LEN;
    s.dir          = random(6);
    s.body[0]      = start;
    s.frame        = 0;
    s.dropped_tail = false;
    s.next_head    = (uint8_t)snake_next_field(start, s.dir);
}

void snake_step(SNAKE &s) {
    // randomly nudge target length within [MIN, MAX]
    if(random(100) < SNAKE_VARY_PROB) {
        if(random(2) && s.target_len < SNAKE_MAX_LEN)      s.target_len++;
        else if(s.target_len > SNAKE_MIN_LEN)              s.target_len--;
    }

    // a tail drops whenever we're not growing (len >= target)
    s.dropped_tail = (s.len >= s.target_len);
    if(s.dropped_tail) s.old_tail = s.body[s.len-1];

    if(s.len < s.target_len)      s.len++;   // grow
    else if(s.len > s.target_len) s.len--;   // shrink

    for(int i=s.len-1; i>0; i--) s.body[i] = s.body[i-1];
    s.body[0] = s.next_head;

    if(random(100) < SNAKE_TURN_PROB) s.dir = random(6);
    s.next_head = (uint8_t)snake_next_field(s.body[0], s.dir);
    s.frame = 0;
}


// --- reshuffle: called every time the mode is selected ---

void snake_reshuffle() {
    num_snakes   = (uint8_t)(random(2, SNAKE_MAX_COUNT+1));
    snake_mirror = 3; //random(1,4);
    for(int s = 0; s < num_snakes; s++)
        snake_init(snakes[s], (uint8_t)(s * NUM_LEDS / num_snakes));
    p("snake reshuffle: n="); p(num_snakes);
    p(" mirror="); pl(snake_mirror);
}


// --- render ---

void snake_render(SNAKE &s, uint8_t h) {
    uint8_t fade_in  = (uint8_t)(255UL * s.frame / SNAKE_MOVE_FRAMES);
    uint8_t fade_out = 255 - fade_in;

    // fading tail first so body pixels win on overlap
    if(s.dropped_tail) snake_paint(s.old_tail, h, fade_out);
    snake_paint(s.next_head, h, fade_in);
    for(int i=0; i<s.len; i++) snake_paint(s.body[i], h, 255);
}

void snake_loop() {
    for(int l=0; l<NUM_LEDS; l++) leds[l] = CRGB::Black;

    uint8_t snake_hues[3] = { (uint8_t)hue, (uint8_t)hue_a, (uint8_t)hue_b };
    for(int s=0; s<num_snakes; s++) {
        uint8_t h = snake_hues[s % 3];
        snakes[s].frame++;
        if(snakes[s].frame >= SNAKE_MOVE_FRAMES) snake_step(snakes[s]);
        snake_render(snakes[s], h);
    }

    FastLED.show();
    delay(20);
}
