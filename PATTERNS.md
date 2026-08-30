# DiscLamp Patterns

The lamp is a hexagonal disc of 151 NeoPixels across 15 rows (4–13–4 profile), roughly circular, ~12 units across. Coordinates go from about −6 to +6 on both axes with hex grid spacing (DY=0.866). Most patterns respond to a global `hue` and `invert` flag.

---

## Static / Shape Patterns

### `full`
Every LED lit at the current hue. Solid-colour disc, no animation.

### `half`
Left or right half of the disc lit, split at x=0. `invert` flips which half is on — gives a two-tone look.

### `square`
A large filled square centred on the disc (±5 units). `invert` shows only the border ring outside the square.

### `triangle`
An upward-pointing triangle filling roughly the lower two-thirds of the disc, defined by two angled lines meeting at the bottom.

### `cross` *(currently disabled in main)*
A plus/cross shape: a wide vertical bar (±2.1) and a narrower horizontal bar (±1.8). `invert` shows everything except the cross.

### `lines`
Parallel diagonal stripes running at roughly 30° (x + y offset), repeating every 3 units. `invert` swaps lit/dark. Static, no movement.

### `line`
A single soft diagonal line at ~40° angle across the disc, with a gradient falloff over ~2 units either side. `invert` darkens the band and lights the surround.

### `stripes`
Smooth sine-wave gradient stripes running diagonally (along x+y). The brightness varies continuously — no hard edges, no animation.

### `hatch`
A crosshatch texture based on a mod-6 row cycle. Alternates between horizontal runs, diagonal offsets, and scattered dots — creates a woven/mesh look. `invert` available.

### `hatch2`
Similar to `hatch` but uses a mod-8 cycle — slightly different spacing, denser feel. `invert` available.

### `arrow`
Repeating chevron/arrow lines pointing upward. Every 3 columns a lit column appears, offset per row to form V-shaped arrows tiling the disc. `invert` available.

### `trif`
A hardcoded triforce — three triangles arranged in a triangle formation, drawn from fixed LED indices. Static, single hue.

---

## Animated Patterns

### `waves`
Concentric brightness bands that sweep outward from the centre, using each LED's row index as the phase. Bands move slowly (f increments every 5 frames). `invert` makes the bright band a dark band on a bright field.

### `scroll_lines`
The same diagonal stripe geometry as `lines` but the stripes scroll sideways continuously (f increments every frame). Smooth drifting parallel lines. `invert` available.

### `zig`
V-shaped zigzag bands that scroll upward. The pattern mirrors left/right (|x| symmetry), so lines converge at the centre forming chevrons. Faster animation than `scroll_lines`.

### `rain`
10 glowing particles fall from the top of the disc to the bottom. Each is a soft dot with exponential brightness falloff (~radius 1). When a particle exits the bottom it resets to a random x position at the top. Looks like falling embers or raindrops.

### `hues`
Three glowing blobs (at three hues spaced 120° apart on the colour wheel) drift around the disc using a physics-based orbit system: velocity + acceleration + edge bouncing + random impulses. The blobs add together with RGB mixing so overlapping areas blend colours. Sizes differ (radii 10, 7, 0.4) so one fills most of the disc, one is mid-size, one is a small accent.

### `automaton`
A hexagonal cellular automaton (hex Game of Life variant). Starts from a small seed cluster in the centre and evolves every 2 seconds using 3 neighbour-count rules chosen randomly. Live cells glow and colour-cycle between `hue` and `hue_a` (offset by 50); dead cells fade out gradually. If the pattern dies out or gets stuck in a loop, the automaton resets or picks new rules automatically.

### `snake`
Two snakes wander the hex grid simultaneously, each with its own hue (`hue` and `hue_a`). The head fades in and the tail fades out smoothly over each move cycle (`SNAKE_MOVE_FRAMES`). Each snake has a target length that drifts randomly over time (`SNAKE_VARY_PROB=10%`, range `SNAKE_MIN_LEN`–`SNAKE_MAX_LEN`), so snakes grow and shrink gradually. Direction changes randomly with `SNAKE_TURN_PROB`.

**Symmetry:** controlled by `snake_mirror` — `MIRROR_NONE`, `MIRROR_X` (reflect horizontally, x → −x), or `MIRROR_Y` (reflect vertically, y → −y). When a mirror is active every painted pixel is also drawn at its mirrored position, so each snake appears as a symmetric twin pair. Only one axis at a time.

---

### `clock`
A binary clock. The seconds, minutes, and hours are each encoded as binary bits, with each bit shown as a symmetric pair of LEDs mirrored around a centre point on different horizontal rows:
- Seconds: 6 bits around row centre (LED 75)
- Minutes: 6 bits mirrored on two rows (LEDs 50 and 100)
- Hours: 4 bits on two outer rows (LEDs 26 and 124)

Updates once per second. Reads cleanly once you know the encoding but is more decorative than legible at a glance.

---

## Pattern Ideas / Gaps

- **Radial sweep** — a bright spoke rotating around the centre like a radar or clock hand
- **Colour gradient fill** — hue varies smoothly by radius or angle rather than uniform colour
- **Spiral** — a logarithmic or Archimedean spiral, static or animated rotating
- **Pulse** — the whole disc fades in/out (breathe effect), optionally with a colour drift
- **Noise / fire** — Perlin or simplex noise mapped to brightness/hue for organic texture
- **Ring ripples** — multiple concentric rings expanding outward from a tap/trigger, like water ripples
- **Twinkle** — random LEDs flicker independently at different rates
- **Binary counter** — counts upward in binary across all LEDs, as a data-art pattern
- snake - make each snake have its own symmetry and snakes come and go. maybe when they leave the circle a new snake is added.