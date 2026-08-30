#pragma once
#include <Arduino.h>
#include "pattern.h"
#include "pixel.h"

#define TREE_STEM_MOVE_FRAMES 6     // frames per stem growth step (delay(20) -> ~0.12 s)
#define TREE_MOVE_FRAMES  40    // frames per branch growth step (delay(20) -> ~1 s)
#define TREE_PAUSE_FRAMES 150   // hold finished tree 5 s before regrowing

#define TREE_GROWERS 2           // branches growing simultaneously - raise for more

// directions (subset of snake's): 0=left 1=right 2=up-R 3=up-L 4=dn-R 5=dn-L

#define TREE_STEM    0
#define TREE_BRANCH  1

// one independent growth process: grows from `tip` into `next` (which fades
// in); when it dead-ends it respawns somewhere else on the tree
typedef struct {
    uint8_t  tip;        // committed tree cell it grows out of
    uint8_t  dir;        // direction of last committed growth step
    int      next;       // pixel fading in (-1 = none / dead end)
    uint8_t  next_dir;   // direction tip -> next
    int16_t  frame;      // counts up to move_frames while next fades in;
                         // starts negative to stagger parallel growers
    bool     active;     // false = idle slot / tree complete
} TREE_GROWER;

typedef struct {
    uint8_t  pos[NUM_LEDS];  // all tree cells; [len-1] = last committed
    uint16_t len;
    uint8_t  mode;           // TREE_STEM or TREE_BRANCH
    uint16_t pause;          // frames left holding the finished tree
    TREE_GROWER g[TREE_GROWERS];
} TREE;

TREE tree;


// turn rule: every step must turn +/-60 deg, never straight
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


// --- growth rules ---
// a new cell must not touch the existing tree, except for the cell
// it grows out of - keeps branches one empty field apart.
// other growers' pending cells count as tree, so parallel tips never
// collide or grow into each other

bool tree_adjacent(uint8_t a, uint8_t b) {
    int dhx = pixel[a].hx - pixel[b].hx;
    int dhy = pixel[a].hy - pixel[b].hy;
    if(dhy == 0)              return dhx ==  2 || dhx == -2;  // same row
    if(dhy == 1 || dhy == -1) return dhx ==  1 || dhx == -1;  // diagonal
    return false;
}

bool tree_branch_valid(int c, uint8_t from, uint8_t gi) {
    if(c < 0) return false;
    for(uint16_t i=0; i<tree.len; i++) {
        if(tree.pos[i] == c)              return false;  // already tree
        if(tree.pos[i] == from)           continue;      // allowed parent
        if(tree_adjacent(tree.pos[i], c)) return false;  // touches tree
    }
    for(uint8_t i=0; i<TREE_GROWERS; i++) {
        if(i == gi || !tree.g[i].active || tree.g[i].next < 0) continue;
        if(tree.g[i].next == c)              return false;  // other tip's target
        if(tree_adjacent(tree.g[i].next, c)) return false;  // keep distance
    }
    return true;
}

// continue growing: turn +/-60 deg from the last direction, if valid
int tree_pick_branch(uint8_t idx, uint8_t dir, uint8_t &ndir, uint8_t gi) {
    uint8_t opts[2];
    tree_branch_options(dir, opts);

    int     cand[2];
    uint8_t cdir[2];
    uint8_t nc = 0;
    for(uint8_t i=0; i<2; i++) {
        int c = tree_next_field(idx, opts[i]);
        if(tree_branch_valid(c, idx, gi)) { cand[nc] = c; cdir[nc] = opts[i]; nc++; }
    }
    if(nc == 0) return -1;          // dead end: this tip is finished

    uint8_t pick = random(nc);
    ndir = cdir[pick];
    return cand[pick];
}

// first step of a new growth (stem or branch): try all 6 directions in random order
int tree_branch_start(uint8_t idx, uint8_t &ndir, uint8_t gi) {
    uint8_t order[6] = {0,1,2,3,4,5};
    for(uint8_t i=5; i>0; i--) {
        uint8_t j = random(i+1);
        uint8_t t = order[i]; order[i] = order[j]; order[j] = t;
    }
    for(uint8_t i=0; i<6; i++) {
        int c = tree_next_field(idx, order[i]);
        if(tree_branch_valid(c, idx, gi)) { ndir = order[i]; return c; }
    }
    return -1;
}

// restart delay for grower gi: pick the countdown so this grower's commits
// land halfway between the other grower's commits - the branches then fade
// their pixels in alternately. computed from the other grower's current
// position in its cycle, so it works no matter when the restart happens.
// falls back to an even spread by index when no other grower is running
int16_t tree_restart_delay(uint8_t gi) {
    for(uint8_t i=0; i<TREE_GROWERS; i++) {
        if(i == gi || !tree.g[i].active || tree.g[i].next < 0) continue;
        int16_t d = (int16_t)((TREE_MOVE_FRAMES/2 - tree.g[i].frame) % TREE_MOVE_FRAMES);
        if(d < 0) d += TREE_MOVE_FRAMES;
        return d;
    }
    return (int16_t)((int)gi * TREE_MOVE_FRAMES / TREE_GROWERS);
}

// respawn grower gi on a random tree cell, then scan linearly (wrapping)
// for one that can still grow a branch; false if the tree is complete
bool tree_start_new_branch(uint8_t gi) {
    TREE_GROWER *g = &tree.g[gi];
    uint16_t start = random(tree.len);
    for(uint16_t k=0; k<tree.len; k++) {
        uint16_t i = (start + k) % tree.len;
        uint8_t ndir;
        int c = tree_branch_start(tree.pos[i], ndir, gi);
        if(c >= 0) {
            g->tip      = tree.pos[i];
            g->dir      = ndir;
            g->next     = c;
            g->next_dir = ndir;
            g->frame    = -tree_restart_delay(gi);
            g->active   = true;
            return true;
        }
    }
    return false;
}

// activate a grower in every idle slot (when branch mode begins)
void tree_spawn_growers() {
    for(uint8_t i=0; i<TREE_GROWERS; i++)
        if(!tree.g[i].active)
            tree_start_new_branch(i);
}


// --- state machine ---

// base led: the centre led of the disc
uint8_t tree_base() {
    return (uint8_t)find_pixel(0.0f, 0.0f);
}

void tree_init() {
    tree.len   = 1;
    tree.pos[0]= tree_base();
    tree.mode  = TREE_STEM;
    tree.pause = 0;
    for(uint8_t i=0; i<TREE_GROWERS; i++) tree.g[i].active = false;

    // grower 0 is the stem, started like any other growth
    TREE_GROWER *g = &tree.g[0];
    g->tip      = tree.pos[0];
    g->frame    = 0;
    g->active   = true;
    g->next     = tree_branch_start(g->tip, g->next_dir, 0);
    g->dir      = g->next_dir;
    if(g->next < 0) {
        g->active = false;
        tree.pause = TREE_PAUSE_FRAMES;
    }
}

void tree_reshuffle() {
    tree_init();
}

uint8_t tree_move_frames() {
    return (tree.mode == TREE_STEM) ? TREE_STEM_MOVE_FRAMES : TREE_MOVE_FRAMES;
}

// commit one grower's pending cell, then pick its next target; stem and
// branches share the same pick, a stem that dead-ends (hits the outside)
// fires up all branch growers, a branch respawns elsewhere
void tree_grower_step(uint8_t gi) {
    TREE_GROWER *g = &tree.g[gi];

    tree.pos[tree.len++] = (uint8_t)g->next;
    g->tip   = g->next;
    g->dir   = g->next_dir;
    g->frame = 0;

    g->next = tree_pick_branch(g->tip, g->dir, g->next_dir, gi);
    if(g->next < 0) {
        if(tree.mode == TREE_STEM) {
            tree.mode = TREE_BRANCH;
            g->active = false;
            tree_spawn_growers();
        }
        else if(!tree_start_new_branch(gi))
            g->active = false;           // this grower is done for good
    }
}


// --- render ---

void tree_render() {
    for(uint16_t i=0; i<tree.len; i++) {
        LED_STRUCT q = pixel[tree.pos[i]];
        leds[tree.pos[i]].setHSV( grad_hue(q.x, q.y), 250, 255 );
    }
    for(uint8_t i=0; i<TREE_GROWERS; i++) {
        TREE_GROWER *g = &tree.g[i];
        if(!g->active || g->next < 0 || g->frame <= 0) continue;
        uint8_t fade = (uint8_t)(255UL * g->frame / tree_move_frames());
        LED_STRUCT q = pixel[g->next];
        leds[g->next].setHSV( grad_hue(q.x, q.y), 250, fade );
    }
}

void tree_loop() {
    for(int l=0; l<NUM_LEDS; l++) leds[l] = CRGB::Black;

    if(tree.pause > 0) {
        tree.pause--;
        if(tree.pause == 0) tree_init();   // new base, new stem
    }
    else {
        uint8_t mf = tree_move_frames();
        for(uint8_t i=0; i<TREE_GROWERS; i++) {
            TREE_GROWER *g = &tree.g[i];
            if(!g->active) continue;
            g->frame++;
            if(g->frame >= mf && g->next >= 0) tree_grower_step(i);
        }

        // tree complete when no branch can grow anywhere anymore
        if(tree.mode == TREE_BRANCH) {
            bool any = false;
            for(uint8_t i=0; i<TREE_GROWERS; i++) any |= tree.g[i].active;
            if(!any) tree.pause = TREE_PAUSE_FRAMES;
        }
    }

    tree_render();
    FastLED.show();
    delay(20);
}
