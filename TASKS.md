# New Animations — Tasks

Three new patterns, each built in its own header so they can be developed in parallel.
Wiring is already done: `src/main.cpp` (include + dispatch) and `include/html.h` (mode list)
know about `zap`, `ripple` and `vortex`. Each header currently contains a black stub.

## Shared rules for every task

- **Edit only your own header** (`include/zap.h`, `include/ripple.h`, `include/vortex.h`).
  Do not touch `main.cpp`, `html.h`, `serial.h`, `pattern.h`, `gradient.h` or other patterns.
- Follow the house style (see `ray.h`, `moon.h`, `grid.h`): `#define` tunables block at the top
  with comments, `static` state, one public `void <name>()` called once per frame from `loop()`,
  which renders, calls `FastLED.show()` and ends with a `delay()`.
- Geometry: 151 px hex disc, `pixel[l].x / .y` in roughly −6..+6 (row spacing `DY = 0.866`),
  `pixel[l].hx / hy` integer hex coords (in-row step 2, diagonal step 1). See `include/pixel.h`.
- Colour: use `grad_hue(x, y)` (or `hue` / `hue_a` / `hue_b`) so the web colour slider works.
  Respect `invert` where it makes sense. `param` (0..100) is available if a tunable is handy.
- ESP8266 (d1_mini): keep RAM small (no big float arrays beyond a few × `NUM_LEDS`), prefer
  `sinf/cosf/sqrtf`, aim for ≥ 30 fps with all the per-pixel maths.
- Build check (use your own build dir so parallel builds don't collide):
  `PLATFORMIO_BUILD_DIR=/tmp/claude-1000/pio-<name> ~/.platformio/penv/bin/pio run -e lamp`
- Do not upload, do not commit, do not edit shared files (incl. `PATTERNS.md` / this file).
  When done, report a 2–4 line `PATTERNS.md`-style description of your pattern plus the main
  tunables; it gets merged in afterwards.

---

## 1. `zap` — laser lines crossing  *(from README: "laser lines crossing / zap — lines / stripes of 2/3 width")*

**Effect.** A dark disc with laser beams firing across it. A beam appears as a thin streak
2–3 px wide that *shoots* in from one edge — bright, slightly desaturated head racing ahead —
and leaves a full-length line behind it that holds for a moment, then decays. Several beams at
different angles are alive at once, so they cross; where two beams intersect the crossing point
flares brighter / whiter (a little "zap" spark). Occasionally a beam flickers on ignition like a
striking laser. Calm but energetic — think laser show seen through haze.

**Algorithm.**
- Pool of ~4 `Beam` structs: angle θ (biased toward the 3 hex axes 0°/60°/120° so lines look
  crisp on the grid, with some free angles), perpendicular offset `d` from centre (−4..4),
  width `w` (≈0.6–1.0 so it covers 2–3 px), `head` position along the line (−7 → +7),
  speed, state (`FIRING → HOLD → FADE → DEAD`), age/brightness.
- Per pixel, per beam: distance to line `|nx·x + ny·y − d|`, along-line coordinate
  `t = dx·x + dy·y`. Lit if within width and `t < head`; soft edge `(1 − dist/w)²` like `ray.h`.
  Brighter, lower-saturation band right at the head (`head − t < 1`).
- Accumulate per-pixel brightness over beams; count how many beams hit a pixel —
  ≥ 2 → crossing, push saturation down (toward white) and value up, maybe a brief sparkle.
- Keep a `uint8_t` persistence buffer and fade it each frame (`scale8`) for afterglow, take max
  with the fresh frame.
- Spawn new beams at random intervals when slots free up; vary hue per beam a little
  (`hue`, `hue_a`, `hue_b`) or use `grad_hue`.
- `invert`: dark beams cutting through a lit disc.

---

## 2. `ripple` — raindrops on a pond  *(variation of `circle` / `ring` / `waves`)*

**Effect.** Drops land at random spots on the disc. Each drop makes a small bright splash then
expanding concentric rings that travel outward, thinning and dimming as they go. Multiple drops
overlap and their rings interfere — where crests meet they add up brighter, troughs cancel —
giving a living water-surface shimmer. Occasionally a bigger drop (stronger, slower, more rings).
Quiet, meditative.

**Algorithm.**
- Pool of ~5 `Drop` structs: centre `(cx, cy)` inside the disc, `age` (time since impact),
  amplitude `A`, wavelength `λ` (~2.0–3.0), speed `v`, damping.
- Height at a pixel: for each drop, `r = dist(p, c)`, front `R = v·age`;
  `h += A · cos(2π (r − R)/λ) · env(r − R) · exp(−k·age)` where `env` is a gaussian /
  raised-cosine window behind the front (only ~2 wavelengths of rings) and 0 ahead of it.
- Map height → brightness: `val = base + gain · h` clamped (a dim "water" base level so the
  disc never fully goes black), and use height to nudge hue slightly (crests toward `hue_a`,
  troughs toward `hue_b`) for a subtle colour shimmer.
- Splash: for the first few frames of a drop, add a bright soft dot at the centre.
- Optional: reflect rings off the disc edge (mirror image source at distance `2·Rdisc − |c|`)
  for extra interference — only if it stays fast.
- Spawn: random interval (Poisson-ish), retire drops when amplitude is negligible.
- `invert`: crests dark, troughs bright.

---

## 3. `vortex` — rotating spiral galaxy  *(variation of `scroll_lines` in polar coordinates)*

**Effect.** 2–3 soft spiral arms wind out from the centre and slowly rotate, like a galaxy or a
whirlpool seen from above. The amount of twist "breathes" — arms slowly tighten into a coil then
unwind toward straight spokes — and the rotation drifts in speed and occasionally reverses,
smoothly. Hue shifts along each arm from centre to rim (`hue` → `hue_a`), and the core has a
gentle warm glow. Hypnotic, slow.

**Algorithm.**
- Per pixel: polar `r = sqrt(x²+y²)`, `a = atan2f(y, x)` — these never change, so precompute
  `r` and `a` once into small static arrays (2 × 151 floats is fine).
- Spiral phase: `φ = N·(a − rot) + twist · r` (Archimedean) or `twist · logf(r + 1)`
  (logarithmic — test which looks better on 12 px across). `N` = number of arms.
- Brightness: `val = pow(0.5 + 0.5·cos φ, sharp)`, `sharp` ~2–4 to make the arms thinner than
  the gaps; multiply by a radial falloff so arms fade slightly toward the rim, plus
  `core = exp(−r²/σ²)` added at the centre.
- Motion: `rot += ω` each frame; `ω` eased toward a randomly chosen target every few seconds
  (like `scroll_lines_tick`), sometimes negative. `twist` oscillates slowly between ~0.3 and
  ~1.5 rad/unit (sin over ~20 s) plus small random drift. Occasionally change `N` (2 ↔ 3) via a
  crossfade between the two fields rather than a hard switch.
- Hue: `hue + (hue_a − hue)·(r / 6)` or `grad_hue`; core slightly toward `hue_b`.
- `invert`: dark arms on a lit disc.

---

# Round 2 — something genuinely new

Same shared rules as above. Stubs `include/sand.h` and `include/firefly.h` are already wired
into `main.cpp` and `html.h`.

## 4. `sand` — tilting hourglass of falling grains  *(physics sim — nothing like it yet)*

**Effect.** The disc is a sealed glass full of coloured sand: ~40–60% of the pixels are grains,
the rest empty space. Gravity points "down"; grains tumble, slide and pile up in a heap against the
rim. Every ~20–40 s the lamp is "tilted": gravity rotates by 60° (one hex direction) or flips 180°,
and the whole heap avalanches across the disc into a new resting pile, grains cascading over each
other. Once settled, it rests (perfectly still, maybe a faint shimmer) until the next tilt. Grains
keep their own colour (a few bands of hue picked at start, e.g. `hue`, `hue_a`, `hue_b`, layered like
sand art), so as the heap tumbles the layers mix and fold — a slow, satisfying, tactile animation.

**Algorithm.**
- Cellular sand on the hex grid. Precompute a neighbour table `nb[NUM_LEDS][6]` (−1 = wall)
  from `pixel[].hx / hy` (in-row step ±2 hx, diagonal steps ±1 hx & ±1 hy). Uses ~900 B, ok.
- Gravity = one of the 6 hex directions `g`. Per step, visit cells in order of "lowest first"
  along g (sort indices by projection of (x,y) onto g once per tilt). For each grain: if the cell
  in direction g is empty → move there; else try the two neighbouring directions (g±60°) in random
  order → slide; else stay. Moving one cell per step gives a falling look.
- Per-cell state: `uint8_t grain[NUM_LEDS]` (0 = empty, else colour index / hue). Optionally a
  small `moved` flag to render a brief brightness boost on moving grains (sparkle while flowing).
- Smooth motion: interpolate brightness between old and new cell over a few frames (like
  `snake.h` / `fire.h` do with prev/next buffers), so grains glide rather than teleport.
- Tilt scheduler: when nothing moved for N frames → wait hold time → pick new g. Mostly ±60°,
  sometimes 180° for a big dramatic cascade.
- Colours: grains coloured by stripe at start (e.g. by initial height band) so layers are visible;
  empty cells dark (or very dim). `invert`: empty space lit, grains dark (air bubbles in liquid).
  `param` could set the fill fraction (re-seed on change via `rain_restart` style is not
  available — just read `param` when seeding/tilting).

## 5. `firefly` — fireflies that learn to blink in sync  *(emergent sync — Kuramoto oscillators)*

**Effect.** Night meadow. ~20–30 fireflies sit scattered over the disc, each blinking its own short,
soft flash (quick rise, slow ~0.5 s fade) on its own rhythm — random, sparse, twinkly. Over 20–60 s
they gradually pull into step: little local clusters sync first, waves of flashes ripple across the
disc, until the whole meadow pulses together in one breath-like flash. It holds for a while, then
a "disturbance" scatters their phases (and a few fireflies fly to new spots) and the slow
self-organisation starts again. Organic, calm, and it visibly *changes character* over a minute.

**Algorithm.**
- `N` fireflies, each: pixel index (or float x,y snapped to nearest pixel, can drift slowly),
  phase `θ ∈ [0,1)`, natural frequency `ω` (period ~1.5–2.5 s, small spread), hue offset.
- Pulse-coupled sync (Mirollo–Strogatz style, simpler & more firefly-like than full Kuramoto):
  each frame `θ += ω·dt`; when θ ≥ 1 → flash, θ = 0, and nudge every firefly within radius `R`
  forward: `θ_j += ε·(1 − θ_j)` or `θ_j = min(1, θ_j·(1+ε))`. Neighbourhood radius gives local
  clusters + travelling waves before global sync. Coupling `ε` ramps up from ~0 over the cycle so
  the sync is gradual, not instant.
- Flash render: brightness from time since last flash: fast attack (~60 ms), exponential decay
  (~400–600 ms); draw as a soft glow on the pixel plus a faint halo on its 6 neighbours.
  Additive blend, colour `hue`/`hue_a` mix, slightly warm-white at peak.
- Background: very dim, faintly varying "grass" glow near the bottom of the disc (optional).
- Cycle state machine: DRIFT (ε rising) → SYNCED (detect: order parameter
  `|Σ e^{2πiθ}| / N > 0.95` held for ~10 s) → SCATTER (randomise phases, relocate a few, reset ε).
- `param`: coupling strength / how fast they sync. `invert`: dark flashes on a lit field.
