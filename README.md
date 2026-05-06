# jr-dly-1: MIDI-Synced Headless Delay Effect

A low-latency, real-time delay effect written in C, slaved to Ableton Live via MIDI Clock. Built as a solo personal backend engineering project for the Boot.dev path.

## Overview

**jr-dly-1** is a CLI-based audio processor that implements a tempo-synced delay with variable feedback and mix control. The effect synchronizes to an external MIDI clock source (Ableton Live) via 24 MIDI clock pulses per quarter note (ppqn), enabling precise beat-aligned delay divisions (1/4, 1/8, 1/16, etc.).

### Key Features

- **MIDI Clock Synchronization**: Captures MIDI clock messages to derive tempo and beat timing
- **Tempo-Synced Delay Divisions**: Select delay times locked to musical subdivisions (quarter note, eighth note, triplets, etc.)
- **Real-Time Audio Processing**: Low-latency circular buffer implementation with lock-free thread synchronization patterns
- **CLI Control**: Adjust mix, feedback, and delay division via terminal input or MIDI CC messages
- **Headless Architecture**: No GUI—fully command-line driven for integration in DAW workflows

## Project Context

**Lead Frontend Engineer & Electronic Music Producer** working on backend fundamentals through the Boot.dev path. This project targets **20–40 hours** and prioritizes:

- Real-time thread safety and synchronization primitives
- Manual memory management (circular buffer allocation and pointer arithmetic)
- MIDI protocol parsing and clock-to-BPM conversion
- Audio callback implementation and sample-accurate timing

## Tech Stack

| Component         | Library/Tool                                                       |
| ----------------- | ------------------------------------------------------------------ |
| **Language**      | C (C11/C99 standard)                                               |
| **Audio I/O**     | [miniaudio.h](https://miniaud.io/) (single-header, cross-platform) |
| **MIDI Handling** | RtMidi (C wrapper for MIDI input)                                  |
| **Interface**     | CLI (stdin for parameter control)                                  |
| **Build System**  | Makefile                                                           |

## Project Plan: 40-Hour Roadmap

### Phase 1: Foundation & Audio Setup (8 hours)

**Objective**: Establish real-time audio pipeline and circular buffer infrastructure.

1. **Audio Callback Architecture (3 hours)**
   - Initialize miniaudio device with callback function
   - Implement audio buffer management (sample rate, frame count)
   - Set up error handling for real-time constraints
   - Test latency profile

2. **Circular Buffer Implementation (4 hours)**
   - Design fixed-size circular buffer (heap-allocated)
   - Implement write/read pointer management with pointer arithmetic
   - Handle buffer wraparound edge cases
   - Unit test with synthetic signals (sine wave)

3. **Basic DSP Plumbing (1 hour)**
   - Wet/dry mix parameter structure
   - Signal flow: input → delay buffer → output
   - Validate signal passes through without distortion

**Deliverables**: Compiling project with audio I/O working; buffer read/write verified.

---

### Phase 2: MIDI Clock & BPM Calculation (8 hours)

**Objective**: Accurately derive tempo from MIDI clock and convert to sample-accurate timing.

1. **RtMidi Integration (2 hours)**
   - Initialize MIDI input port and select Ableton Live source
   - Implement MIDI message callback (status/velocity parsing)
   - Filter for MIDI clock messages (0xF8)
   - Handle MIDI start/stop/continue (0xFA, 0xFC, 0xFB)

2. **MIDI Clock to BPM Conversion (3 hours)**
   - Track clock tick timestamps (high-resolution timer)
   - Calculate BPM from 24 ppqn rate: `BPM = (ticks_per_minute / 24)`
   - Implement running average filter to smooth tempo jitter
   - Calculate samples-per-beat at current sample rate

3. **Delay Division Timing (3 hours)**
   - Map division modes (1/4, 1/8, 1/16, 1/8T, etc.) to sample counts
   - Implement phase-locked delay: trigger on MIDI beat boundaries
   - Synchronize read pointer to beat grid
   - Test with Ableton's metronome

**Deliverables**: BPM display in CLI; delay time locked to beat divisions with <5ms drift.

---

### Phase 3: Real-Time Signal Processing (12 hours)

**Objective**: Implement feedback delay with thread-safe parameter updates.

1. **Feedback Delay Algorithm (4 hours)**
   - Implement single-tap delay with feedback: `y[n] = x[n] + α·y[n-d]`
   - Parameter: feedback (0.0–0.95, clip to prevent instability)
   - Handle denormal numbers (flush to zero)
   - Anti-aliasing considerations at high feedback

2. **Wet/Dry Mix & Output Scaling (2 hours)**
   - Mix parameter: 0.0 (fully dry) to 1.0 (fully wet)
   - Gain correction: prevent doubling on 50/50 mix
   - Prevent clipping: monitor headroom and soft limiting if needed

3. **Thread Safety & Lock-Free Updates (6 hours)**
   - Identify shared state: delay buffer, feedback coefficient, mix parameter
   - Implement atomic reads/writes for parameter changes from CLI
   - Use double-buffering or lock-free queue for parameter updates
   - Test race conditions with thread sanitizer

**Deliverables**: Working feedback delay; smooth parameter changes under real-time constraints; no glitches on feedback updates.

---

### Phase 4: CLI & Parameter Control (6 hours)

**Objective**: Build user interface for real-time parameter adjustment.

1. **Terminal Input Handling (2 hours)**
   - Non-blocking stdin for CLI commands
   - Parse commands: `mix <0.0-1.0>`, `feedback <0.0-0.95>`, `division <1/4|1/8|1/16|...>`
   - Display current state and DSP stats (CPU load, buffer utilization)

2. **MIDI CC Control (2 hours)**
   - Map CC values (0–127) to parameter ranges
   - CC 7 → Mix, CC 74 → Feedback, CC 12 → Division select
   - Smooth CC value filtering to prevent clicks

3. **Status Display & Diagnostics (2 hours)**
   - Real-time BPM, delay time (ms), current division
   - CPU profiling: callback execution time
   - Buffer underrun/overrun detection
   - Help text and usage guide

**Deliverables**: Fully functional CLI; responsive to parameter changes; clean, informative display.

---

### Phase 5: Testing, Optimization & Polish (6 hours)

**Objective**: Validate real-time behavior, optimize performance, document code.

1. **Performance Testing (2 hours)**
   - Measure callback latency under various buffer sizes
   - Verify no allocation/deallocation in real-time callback
   - Test with high feedback and dense MIDI clock rates
   - Profile memory usage

2. **Edge Cases & Robustness (2 hours)**
   - Ableton disconnect/reconnect handling
   - Parameter boundary validation
   - Buffer underrun recovery
   - Graceful shutdown

3. **Documentation & Code Review (2 hours)**
   - Inline comments for pointer arithmetic and circular buffer logic
   - Function documentation (pre/post conditions)
   - Build/run instructions
   - Known limitations and future improvements

**Deliverables**: Tested, optimized binary; comprehensive README and source comments.

---

## Getting Started

### Prerequisites

- macOS or Linux with audio I/O support
- GCC or Clang (C11 support)
- Ableton Live (or compatible MIDI clock source)
- RtMidi development headers

### Build

```bash
make
```

Output: `bin/jr-dly-1`

### Run

```bash
./bin/jr-dly-1
# or
make run
```

The application will:
1. Initialize audio input/output
2. Listen for MIDI clock on the default MIDI input
3. Present a CLI prompt for parameter control

### Example CLI Session

```
jr-dly-1> status
BPM: 120.0 | Delay: 500ms (1/4) | Mix: 0.5 | Feedback: 0.6

jr-dly-1> mix 0.7
Mix set to 0.7

jr-dly-1> feedback 0.8
Feedback set to 0.8

jr-dly-1> division 1/8
Delay division: 1/8 note (250ms)
```

## Learning Objectives

This project emphasizes backend engineering fundamentals:

- **Real-time Constraints**: Understanding callback deadlines and avoiding blocking operations
- **Thread Safety**: Lock-free synchronization for parameter updates without glitches
- **Memory Management**: Manual allocation for circular buffers; pointer arithmetic for ring buffer indexing
- **MIDI Protocol**: Parsing clock messages and converting timing data to musical tempo
- **Signal Processing**: Feedback delay implementation with stability considerations
- **Audio Systems**: Latency, sample rates, and buffer management at the hardware level

## Architecture Notes

### Circular Buffer

The delay line is implemented as a fixed-size heap-allocated buffer with separate read and write pointers. Wraparound is handled via modulo arithmetic:

```
write_ptr = (write_ptr + 1) % buffer_size
read_ptr = (write_ptr - delay_samples) % buffer_size
```

### MIDI Clock Synchronization

- 24 MIDI clock messages per quarter note (standard)
- BPM derived from inter-clock timestamps
- Delay time converted to sample count at current sample rate
- Read pointer phase-locked to beat grid via MIDI start/stop messages

### Thread Model

- **Audio Callback**: Real-time thread (highest priority)
- **MIDI Handler**: Separate thread for MIDI input
- **CLI Input**: Blocking thread for user input

Parameters updated via atomic writes or lock-free queue.

## Known Limitations & Future Work

- Single-tap delay (future: multi-tap, reverb modes)
- No preset save/load
- CLI only (future: OSC for networked control)
- Fixed feedback architecture (future: modulated feedback)

## References

- [miniaudio.h Documentation](https://miniaud.io/)
- [RtMidi GitHub](https://github.com/thestk/rtmidi)
- [MIDI Clock Specification](https://www.sweetwater.com/sweetcare/articles/what-is-midi-clock/)
- [Real-Time Audio Programming in C](http://www.rossbencina.com/code/real-time-audio-programming-101-time-waits-for-nothing)

---

**Project Status**: In Development  
**Target Completion**: 40 hours (Backend Path, Boot.dev)
