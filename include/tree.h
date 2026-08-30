#pragma once
#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

#define TREE_MOVE_FRAMES  50    // frames per growth step (delay(20) -> ~1 s)
#define TREE_PAUSE_FRAMES 100   // hold finished stem 2 s before regrowing

// directions (subset of snake's): 0=left 1=right 2=up-R 3=up-L
// stem never grows down, so 4=dn-R 5=dn-L are never used

typedef struct {
    uint8_t  pos[NUM_LEDS];  // [0]=base .. [len-1]=tip; pixel indices
    uint16_t len;
    uint8_t  dir;            // direction of last committed growth step
    int      next;           // pixel fading in (-1 = none / dead end)
    uint8_t  next_dir;       // direction head -> next
    uint16_t frame;          // counts 0..TREE_MOVE_FRAMES between steps
    uint16_t pause;          // frames left holding the finished stem
} TREE;

TREE tree;


// --- turn rule: every step must turn +/-60 deg, never straight ---
// after L  -> UL          (DL forbidden, stem goes up)
// after R  -> UR          (DR forbidden)
// after UL -> UR or L
// after UR -> UL or R
uint8_t tree_options(uint8_t dir, uint8_t out[2]) {
    uint8_t n = 0;
    switch(dir) {
        case 0: out[n++] = 3;             break;
        case 1: out[n++] = 2;             break;
        case 2: out[n++] = 3; out[n++] = 1; break;
        case 3: out[n++] = 2; out[n++] = 0; break;
    }
    return n;
}

// neighbour lookup, no wrapping: -1 if there is no led there
// NB: L/R must be resolved via x, not index - the strips run in alternating
// directions per row, so idx+1 is only "right" on every second row!
int tree_next_field(uint8_t idx, uint8_t dir) {
    LED_STRUCT pix = pixel[idx];
    switch(dir) {
        case 0: return find_pixel(pix.x-1.0f, pix.y);
        case 1: return find_pixel(pix.x+1.0f, pix.y);
        case 2: return find_pixel(pix.x+0.5f, pix.y+DY);
        case 3: return find_pixel(pix.x-0.5f, pix.y+DY);
    }
    return -1;
}

// pick a random existing option; also reports which direction was taken
int tree_pick_next(uint8_t idx, uint8_t dir, uint8_t &ndir) {
    uint8_t opts[2];
    uint8_t n = tree_options(dir, opts);

    int     cand[2];
    uint8_t cdir[2];
    uint8_t nc = 0;
    for(uint8_t i=0; i<n; i++) {
        int c = tree_next_field(idx, opts[i]);
        if(c >= 0) { cand[nc] = c; cdir[nc] = opts[i]; nc++; }
    }
    if(nc == 0) return -1;          // dead end: stem is finished

    uint8_t pick = random(nc);
    ndir = cdir[pick];
    return cand[pick];
}


// --- state machine ---

// base led: random pick of the two middle leds of the bottom row (x = +/-0.5)
uint8_t tree_random_base() {
    int a = find_pixel( 0.5f, pixel[0].y);
    int b = find_pixel(-0.5f, pixel[0].y);
    if(a < 0) return (uint8_t)b;
    if(b < 0) return (uint8_t)a;
    return random(2) ? (uint8_t)a : (uint8_t)b;
}

void tree_set_next() {
    tree.next = tree_pick_next(tree.pos[tree.len-1], tree.dir, tree.next_dir);
    if(tree.next < 0)
        tree.pause = TREE_PAUSE_FRAMES;   // stuck at the edge: hold, then regrow
}

void tree_init() {
    tree.len   = 1;
    tree.pos[0]= tree_random_base();
    tree.frame = 0;
    tree.pause = 0;

    // first growth step is up-diagonal only (no sideways step off the base)
    int a = tree_next_field(tree.pos[0], 2);   // UR
    int b = tree_next_field(tree.pos[0], 3);   // UL
    if      (a < 0) { tree.next = b; tree.next_dir = 3; }
    else if (b < 0) { tree.next = a; tree.next_dir = 2; }
    else if (random(2)) { tree.next = a; tree.next_dir = 2; }
    else            { tree.next = b; tree.next_dir = 3; }
    tree.dir = tree.next_dir;
    if(tree.next < 0)
        tree.pause = TREE_PAUSE_FRAMES;
}

void tree_reshuffle() {
    tree_init();
}

void tree_step() {
    tree.pos[tree.len++] = (uint8_t)tree.next;
    tree.dir = tree.next_dir;
    tree.frame = 0;
    tree_set_next();
}


// --- render ---

void tree_render() {
    for(uint16_t i=0; i<tree.len; i++) {
        LED_STRUCT q = pixel[tree.pos[i]];
        leds[tree.pos[i]].setHSV( grad_hue(q.x, q.y), 250, 255 );
    }
    if(tree.next >= 0) {
        uint8_t fade = (uint8_t)(255UL * tree.frame / TREE_MOVE_FRAMES);
        LED_STRUCT q = pixel[tree.next];
        leds[tree.next].setHSV( grad_hue(q.x, q.y), 250, fade );
    }
}

void tree_loop() {
    for(int l=0; l<NUM_LEDS; l++) leds[l] = CRGB::Black;

    if(tree.pause > 0) {
        tree.pause--;
        if(tree.pause == 0) tree_init();   // new base, new stem
    }
    else {
        tree.frame++;
        if(tree.frame >= TREE_MOVE_FRAMES && tree.next >= 0) tree_step();
    }

    tree_render();
    FastLED.show();
    delay(20);
}
