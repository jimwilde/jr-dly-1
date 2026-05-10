# jr-dly-1

A beat-synced delay effect that connects directly to Ableton Live via **Ableton Link Audio v4**. Audio flows over the Link session — no virtual audio devices or MIDI required. The delay time locks to the session tempo and can be set in musical beat divisions.

## How it works

Link Audio v4 lets peers in a Link session exchange audio streams with beat-accurate timing metadata. This app:

1. Announces a **"Delay Output"** channel on the Link session (a sink)
2. Subscribes to an audio channel you select from Ableton (a source)
3. Receives audio buffers from Ableton, applies the delay effect, and commits the processed audio back to its sink
4. Ableton picks up "Delay Output" on a receive track

The delay time is calculated from the live session tempo on every buffer, so it stays locked to beat even when Ableton's BPM changes.

## Build

Requires Clang with C++17 and the Ableton Link v4 library (included in `libs/link`).

```bash
make
```

Output: `bin/jr-dly-1`

```bash
make run
# or
./bin/jr-dly-1
```

## Connecting to Ableton Live

**Prerequisites:** Ableton Live 12.4 or later with Link and Link Audio both enabled in Preferences → Link/Tempo/MIDI.

### Signal chain

```
Ableton audio track  →  Link Audio  →  jr-dly-1 (delay applied)  →  Link Audio  →  Ableton receive track
```

### Step-by-step setup

**In Ableton Live:**

1. Enable **Link** and **Link Audio** in Preferences → Link/Tempo/MIDI
2. Create an **Audio Track** with your audio source (e.g. a drum loop)
   - This track's audio is automatically broadcast via Link Audio
3. Create a second **Audio Track** to receive the processed audio
   - Set **Audio From** to `jr-dly-1 / Delay Output`
   - Set **Monitor** to **In**
   - Route to your master

> **Return tracks cannot be used as the receive track** — Live does not expose an Audio From selector on return tracks. Use a regular audio track for both source and receive.

**In jr-dly-1:**

4. Run `./bin/jr-dly-1`
5. Type `l` to list available Link Audio channels from Ableton
6. Type `c <N>` to connect to your source track's channel
7. Audio now flows: Ableton source → delay → "Delay Output" → Ableton receive track

### Avoiding feedback

Keep the source track and receive track completely isolated:

- Source track: Monitor **Off** (plays from clip, not from input)
- Receive track: **Zero sends** — do not route it back to any bus the source track can hear

## Commands

| Command | Description |
|---------|-------------|
| `b` | Toggle bypass (passes audio through unprocessed) |
| `v <0.0–1.0>` | Volume — gain of the delayed signal |
| `f <0.0–0.99>` | Feedback — how much of the delayed signal feeds back into the buffer |
| `w <0.0–1.0>` | Dry/wet mix — `0.0` = dry only, `1.0` = delayed signal only |
| `t <beats>` | Delay time in beats — accepts integers (`1`, `2`) or fractions (`1/2`, `1/4`) |
| `z <ms>` | Manual latency compensation — subtracts this many milliseconds from the delay time so the echo lands on the beat despite round-trip latency |
| `l` | List available Link Audio channels on the session |
| `c <N>` | Connect to channel N from the list |
| `d` | Disconnect the current source channel |
| `s` | Show status (BPM, peers, delay, latency) |
| `m` | Show this command menu |
| `q` | Quit |

### Parameter notes

**Delay time (`t`)** is expressed in beats relative to the Link session tempo. Examples at 120 BPM:

| Command | Division | Delay time |
|---------|----------|------------|
| `t 4` | Whole note | 2000 ms |
| `t 2` | Half note | 1000 ms |
| `t 1` | Quarter note | 500 ms |
| `t 1/2` | Eighth note | 250 ms |
| `t 1/4` | Sixteenth note | 125 ms |

**Latency compensation (`z`)** — because audio travels Ableton → app → Ableton, the echo lands slightly late. The `buffer` value shown by `s` is the base round-trip estimate (two audio buffer lengths). Use that as your starting point for `z`, then fine-tune by ear. Example: if `s` shows `buffer 10.7 ms`, start with `z 10.7` and adjust from there.

## Tests

```bash
make test
```

Tests cover `delay_effect.c` and `menu.c`. `main.c` and `link_bridge.cpp` are excluded — they can't be isolated from the Link Audio runtime.

| File | What's tested |
|------|---------------|
| `tests/delay_effect_test.c` | Init defaults, bypass, dry mix, impulse delay timing, volume scaling, feedback echo decay, dry/wet blend |
| `tests/menu_test.c` | Menu output is non-empty, all 12 commands are described, channels-changed notification content, context pointer is ignored |

Compiled test binaries are removed automatically after the run.

## Architecture

- **`src/main.c`** — CLI loop and command dispatch
- **`src/link_bridge.cpp`** — C++ wrapper around `ableton::LinkAudio`; manages the sink, source subscription, and audio callback
- **`src/delay_effect.c`** — Ring buffer delay with atomic parameters (volume, feedback, mix, bypass, delay time)
- **`include/`** — C headers exposing the bridge and effect to `main.c`
- **`libs/link/`** — Ableton Link v4 (header-only)
