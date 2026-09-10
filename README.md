# Oregon Trail — Hosyond ESP32-S3

An *inspired-by* reimplementation of the Apple II / MECC **Oregon Trail** for the
Hosyond ESP32-S3 2.8" 240×320 capacitive-touch board, in C++ / Arduino
(PlatformIO + LovyanGFX). Landscape (320×240), touch-only, on-screen keyboard,
simple original graphics.

Design reference: `Maxwolf/OregonTrail` (C#). Hardware reference:
`HOSYOND_ESP32S3_TARGET.md`. Full plan: `~/.claude/plans/synchronous-sauteeing-owl.md`.

## Build / flash / watch

The embedded artwork (`src/art/gen/`) is not in the repo — generate it once from a
clone of the C# reference project, whose `Assets/Resources/art` holds the original
MECC PNGs:

```sh
git clone https://github.com/Maxwolf/OregonTrail /tmp/OregonTrail
tools/gen_assets.py /tmp/OregonTrail          # writes src/art/gen/*.{h,cpp}
```

Then:

```sh
pio run -e hosyond-s3                 # build
pio run -e hosyond-s3 -t upload       # flash (board on /dev/ttyACM0)
pio test -e native                    # headless playthrough tests
```

`pio device monitor` needs a TTY — use `tools/mon.py` instead (see below).

## Dev serial commands + screenshots

The dev build takes single-byte commands on the USB serial link:

| byte | action |
|------|--------|
| `S`  | dump the current framebuffer (base64 RGB565) — `tools/screenshot.py` decodes it to PNG |
| `t`  | jump to the title screen |
| `m`  | jump to the main menu |

`tools/mon.py [secs] [--shot-every N]` is a TTY-free monitor (`pio device
monitor` needs a TTY) that streams timestamped serial and can grab a screenshot
timeline over one connection. Opening the port resets the board (native
USB-serial-JTAG), so connect first, then interact.

```sh
~/.platformio/penv/bin/python tools/screenshot.py --out shot.png   # one frame
~/.platformio/penv/bin/python tools/mon.py 30 --shot-every 8       # monitor + shots
```

## Layout

| Path | What |
|------|------|
| `src/hw/` | `Display` (LGFX panel config), `Touch` (FT6336 + calibration), `Storage` (LittleFS), `Screenshot` |
| `src/ui/` | `ScreenStack`, `Screen`, `Theme`, `Widgets` (`MenuList`, buttons, wrapped text) |
| `src/screens/` | one file per view |
| `src/game/` | *(from Phase 3)* pure-C++ simulation, also builds host-side for tests |
| `tools/` | host-side dev scripts |

## Status

- **Phase 1 (done, verified on hardware):** display (landscape 320×240), touch +
  calibration ported from gridiron-esp32s3 (raw FT6336 reads, blocking 4-corner
  cal, NVS storage, edge-nudge), screen stack, title + main-menu screens,
  screenshot + dev-command tooling. Touch accuracy confirmed by interactive test.
- **Phase 2 (done, verified on hardware):** new-game flow — profession → start
  month → name the party (on-screen keyboard + preset picker + random) → general
  store with purchase math → outfit summary. `src/game/` model + store pricing +
  session/RNG.
- **Phase 3 (done, verified on hardware):** the travel loop — `game/Trail` +
  `game/Climate` + `game/Sim` turn engine (mileage, daily food, a health model,
  weather, arrival, win/lose); trail menu, animated traveling screen, supplies,
  map, rest, pace/rations, landmark arrival, basic game-over. Headless
  playthrough test in `test/test_playthrough/` (`pio test -e native`).
- **Phase 4 (done, verified on hardware):** trail as a node graph with the three
  historic forks (Fort Bridger vs. Green River ford; the Fort Walla Walla
  detour; Barlow toll road vs. floating the Columbia) — `ForkScreen`; simple
  vector landmark art (`LandmarkArt.h` — forts, spires, domes, rivers,
  mountains); re-entering the store at forts with distance-based price markup;
  a trailside-advice pool.
- **Phase 5 (done, verified on hardware):** weighted random events
  (`game/Events`) — illness/death, ox lost, broken wagon part (fit a spare or
  camp and repair), thief, storms/hail/fog, lost trail, stuck in mud, lucky
  finds — with `EventScreen` and the repair choice. Wired into the turn engine;
  the headless playthrough now runs ~181 event-filled days.
- **Phase 6 (done, verified on hardware):** river crossings (`game/Rivers`,
  `RiverScreen`) — ford / caulk & float / ferry / wait, on the five real
  crossings with the original's depths and widths; spring runoff (weather
  wetness) makes them worse; mishaps flood the wagon, drown oxen, injure or
  drown people.
- **Phase 7 (done, verified on hardware):** the hunting minigame (`HuntScreen`),
  adapted from the disassembled `& HUNT` — 8-compass-point aim where swinging
  round costs time (short way, paced), tap to aim + shoot, wandering
  rabbits/deer/bison, a hunt timer, 100 lb carry cap, every shot spends a round.
  A "Hunt for food" row on the trail menu when the wagon has ammo.
- **Phase 8 (done, verified on hardware):** end-game scoring (`game/Scoring` —
  the `FinalPoints.cs` formula: party health + inventory, times the profession
  multiplier), the Oregon Top Five (`HighScoresScreen`), and resume-after-power-loss
  — the journey is snapshotted to LittleFS at every stop and "Continue journey"
  restores it (`src/SaveGame`).
- **Phase 9 — graphics overhaul (done):** the original MECC Oregon Trail
  artwork, embedded as PNG in flash (~190 KB) and decoded at runtime by
  LovyanGFX. 18 landmark paintings, the DOS wagon/ox/hunter/animal sprites,
  terrain + scenery tiles for a parallax traveling scene, the route map, the
  "The Oregon Trail" wordmark, the family scene, the tombstone. `src/art/` +
  `tools/gen_assets.py`.
- **Hardware + polish (done):** battery gauge (`hw/Battery`, GPIO9 ADC) in the
  HUD + a full readout on the new **Settings** screen (recalibrate / sleep /
  restart / erase Top Five); backlight-off standby with tap-to-wake and a 4-min
  idle timeout (deep/light sleep both hang this board — target ref §13); a
  drag-to-aim ring on the hunt; event-screen art; MECC artwork credit on the
  title.

## Dev serial commands (current)

`S` screenshot · `T<x>,<y>` inject a tap · `j<n>` teleport to trail node n ·
`e<n>` force event kind n (0 = any) · `x` toggle random events · `t` title ·
`m` menu · `p` profession · `o` month · `n` names · `k` names-in-keyboard ·
`g` store · `r` on the trail · `W` / `L` win / lose
