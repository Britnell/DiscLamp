#pragma once
#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

#define TREE_MOVE_FRAMES  40    // frames per growth step (delay(20) -> ~1 s)
#define TREE_PAUSE_FRAMES 150   // hold finished tree 5 s before regrowing

// directions (subset of snake's): 0=left 1=right 2=up-R 3=up-L 4=dn-R 5=dn-L
// stem never grows down (0-3 only), branches may grow in any direction

#define TREE_STEM    0
#define TREE_BRANCH  1

typedef struct {
    uint8_t  pos[NUM_LEDS];  // all tree cells; [len-1] = growing tip
    uint16_t len;
    uint8_t  mode;           // TREE_STEM or TREE_BRANCH
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

// branch turn rule: same +/-60 deg turns, but any direction incl. down
// cycle L -> UL -> UR -> R -> DR -> DL -> L, take its two neighbours
uint8_t tree_branch_options(uint8_t dir, uint8_t out[2]) {
    switch(dir) {
        case 0: out[0]=3; out[1]=5; break;
        case 3: out[0]=0; out[1]=2; break;
        case 2: out[0]=3; out[1]=1; break;
        case 1: out[0]=2; out[1]=4; break;
        case 4: out[0]=1; out[1]=5; break;
        case 5: out[0]=4; out[1]=0; break;
    }
    return 2;
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
        case 4: return find_pixel(pix.x+0.5f, pix.y-DY);
        case 5: return find_pixel(pix.x-0.5f, pix.y-DY);
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


// --- branch growth ---
// a new branch cell must not touch the existing tree, except for the cell
// it grows out of - keeps branches one empty field apart

bool tree_adjacent(uint8_t a, uint8_t b) {
    int dhx = pixel[a].hx - pixel[b].hx;
    int dhy = pixel[a].hy - pixel[b].hy;
    if(dhy == 0)              return dhx ==  2 || dhx == -2;  // same row
    if(dhy == 1 || dhy == -1) return dhx ==  1 || dhx == -1;  // diagonal
    return false;
}

bool tree_branch_valid(int c, uint8_t from) {
    if(c < 0) return false;
    for(uint16_t i=0; i<tree.len; i++) {
        if(tree.pos[i] == c)              return false;  // already tree
        if(tree.pos[i] == from)           continue;      // allowed parent
        if(tree_adjacent(tree.pos[i], c)) return false;  // touches tree
    }
    return true;
}

// continue a branch: turn +/-60 deg from the last direction, if valid
int tree_pick_branch(uint8_t idx, uint8_t dir, uint8_t &ndir) {
    uint8_t opts[2];
    tree_branch_options(dir, opts);

    int     cand[2];
    uint8_t cdir[2];
    uint8_t nc = 0;
    for(uint8_t i=0; i<2; i++) {
        int c = tree_next_field(idx, opts[i]);
        if(tree_branch_valid(c, idx)) { cand[nc] = c; cdir[nc] = opts[i]; nc++; }
    }
    if(nc == 0) return -1;          // dead end: branch is finished

    uint8_t pick = random(nc);
    ndir = cdir[pick];
    return cand[pick];
}

// first step of a new branch: try all 6 directions in random order
int tree_branch_start(uint8_t idx, uint8_t &ndir) {
    uint8_t order[6] = {0,1,2,3,4,5};
    for(uint8_t i=5; i>0; i--) {
        uint8_t j = random(i+1);
        uint8_t t = order[i]; order[i] = order[j]; order[j] = t;
    }
    for(uint8_t i=0; i<6; i++) {
        int c = tree_next_field(idx, order[i]);
        if(tree_branch_valid(c, idx)) { ndir = order[i]; return c; }
    }
    return -1;
}

// pick a random tree cell, then scan linearly (wrapping) for one that can
// still grow a branch; false if the tree is complete
bool tree_start_new_branch() {
    uint16_t start = random(tree.len);
    for(uint16_t k=0; k<tree.len; k++) {
        uint16_t i = (start + k) % tree.len;
        uint8_t ndir;
        int c = tree_branch_start(tree.pos[i], ndir);
        if(c >= 0) {
            tree.next     = c;
            tree.next_dir = ndir;
            tree.dir      = ndir;
            return true;
        }
    }
    return false;
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
    uint8_t tip = tree.pos[tree.len-1];
    if(tree.mode == TREE_STEM)
        tree.next = tree_pick_next(tip, tree.dir, tree.next_dir);
    else
        tree.next = tree_pick_branch(tip, tree.dir, tree.next_dir);
    if(tree.next >= 0) return;

    // stem reached the top / branch dead-ended -> grow a new branch;
    // if nothing can grow anywhere, the tree is complete
    tree.mode = TREE_BRANCH;
    if(!tree_start_new_branch())
        tree.pause = TREE_PAUSE_FRAMES;
}

void tree_init() {
    tree.len   = 1;
    tree.pos[0]= tree_random_base();
    tree.mode  = TREE_STEM;
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
