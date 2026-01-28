# Architecture

This document explains how the subsystems of native-audio-pipe fit together, why they are designed the way they are, and how data moves from a hardware callback through the graph to an output buffer.

---

## The big picture

```
┌─────────────────────────────────────────────────────────────────┐
│  Platform Driver  (CoreAudio / ALSA / JACK / ASIO / Null)       │
│    calls AudioCallback every ~10 ms                             │
└───────────────────────────────┬─────────────────────────────────┘
                                │  raw float buffers
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  AudioGraph                                                     │
│    ┌──────────────┐   ┌──────────────┐   ┌──────────────┐      │
│    │  Source Node │──▶│  Effect Node │──▶│  Sink Node   │      │
│    │  (Oscillator)│   │  (BiQuad)    │   │  (Output)    │      │
│    └──────────────┘   └──────────────┘   └──────────────┘      │
│         │                    ▲                                   │
│         │  IParameter        │  change callback                 │
│         ▼                    │                                   │
│    ┌──────────────┐   ┌──────────────┐                          │
│    │ FloatParam   │   │ EnumParam    │  ◄── automation /        │
│    │ (frequency)  │   │ (filterType) │      MIDI CC /           │
│    └──────────────┘   └──────────────┘      preset load         │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Serialization  (JsonSerializer / BinarySerializer)             │
│    saves/restores graph topology + all parameter values          │
└─────────────────────────────────────────────────────────────────┘
```

---

## Subsystems

### 1. The node contract — `IAudioNode`

Every piece of audio processing in the engine is an `IAudioNode`. The interface defines four lifecycle stages:

| Method | When | What it means |
|--------|------|---------------|
| constructor | once | Allocate internal state, create parameter objects |
| `prepare(sampleRate, blockSize)` | before first process, or on config change | Cache anything that depends on sample rate (smoothing coefficients, filter coefficients) |
| `process(in, out, numFrames, numChannels)` | every audio block (~10 ms) | The hot path. Must not allocate, must not block |
| `reset()` | on demand | Clear delay lines and internal state back to silence |

The `process()` contract is the most important one. It runs on the real-time audio thread, which on most systems has a hard deadline — if you miss it, you get a glitch. This means:

- **No heap allocation.** No `std::vector` resize, no `new`, no `std::string` construction.
- **No blocking.** No mutex locks, no file I/O, no network calls.
- **No expensive math per sample.** A single `sin()`/`cos()` per block is fine. One per sample is not.

Nodes that need per-sample smoothing (like BiQuadFilter) pre-cache their smoothing coefficients in `prepare()` and use only multiply-add in the loop.

### 2. The graph — `AudioGraph`

`AudioGraph` owns the set of nodes and the connections between them. When `processBlock()` is called, it:

1. Runs `FeedbackLoopDetector` to verify no cycles exist.
2. Runs `ExecutionSorter` to produce a topological processing order — every node's inputs are guaranteed to be ready before it runs.
3. Walks the sorted order, calling `process()` on each node in sequence.

Connections are managed by `ConnectionManager`, which stores directed edges as `(sourceNode, sourceChannel) → (destNode, destChannel)` pairs. The graph supports multi-channel routing — you can connect the left output of one node to the right input of another.

`ExecutionSorter` also exposes `computeParallelGroups()`, which identifies nodes at the same graph depth that have no data dependency on each other. This is the basis for future multi-threaded processing.

### 3. Parameters — `IParameter` and friends

Parameters decouple a node's controllable state from its audio processing. Every knob, switch, or mode selector in the engine is an `IParameter` object owned by the node that uses it.

The hierarchy:

```
IParameter          — name, type, reset
  └── INumericParameter  — getValue/setValue, normalized range, smoothing, callbacks
        ├── FloatParameter   — continuous (frequency, gain, Q)
        └── IntParameter     — discrete (buffer size, tap count)
  EnumParameter      — selection from a string list (filter type, interpolation mode)
  BoolParameter      — on/off toggle
  TriggerParameter   — momentary event (note-on, bypass pulse)
```

Three things make the parameter system work at audio rates:

**Range clamping.** `FloatParameter::setValue()` clamps to `[min, max]` before storing. The node never sees an out-of-range value.

**Smoothing.** `enableSmoothing(true, timeSeconds)` turns on a one-pole filter so that when a value jumps from 1000 Hz to 5000 Hz, the node sees a gradual ramp instead of an instant step. Without this, biquad coefficients would click audibly. The smoothing is advanced per-sample in `process()` using a pre-cached coefficient — no `exp()` in the hot loop.

**Change callbacks.** `setChangeCallback(fn)` registers a function that fires synchronously whenever `setValue()` is called from *any* source — automation, MIDI CC, preset load, UI. The callback typically recomputes internal state (like filter coefficients) so the next `process()` block picks up the new value. This is how external systems talk to nodes without touching the audio thread directly.

### 4. The driver layer — `IAudioDriver`

Drivers are the bridge between the engine and the operating system's audio hardware. They own the audio thread — the driver calls back into user code with input/output buffers at a fixed rate.

```
Driver                          Engine
  │                                │
  │  AudioCallback(in, out, ...)   │
  │ ──────────────────────────────▶│  graph.processBlock()
  │                                │    node1.process()
  │                                │    node2.process()
  │◀──────────────────────────────│
  │  (output buffer filled)        │
```

The `AudioStreamConfig` struct carries sample rate, buffer size, and channel count. Drivers enumerate available devices via `getAvailableDevices()` and configure via `configure(config)`. A state machine (`Uninitialized → Initialized → Running → Stopped`) prevents calling methods out of order.

The `NullAudioDriver` exists specifically for testing — it lets you drive the graph manually without real hardware.

### 5. Serialization — presets and state

Two serialization formats:

- **JSON** — human-readable, suitable for editing by hand or inspecting in version control.
- **Binary** — compact, fast, with a magic number (`0x4E415042`, "NAPB") and format version for forward compatibility.

Both implement `ISerializer` and bind to an `AudioGraph` and a `ParameterGroup`. Serialization captures the full graph topology (which nodes exist, how they are connected) plus every parameter value.

`PresetManager` sits on top and adds filesystem management — scanning a directory for `.nap` preset files, organizing by category, tracking dirty state, and firing load/save callbacks so the UI can update.

`StateVector` is a lightweight snapshot of typed parameter values. It supports `lerp()` (interpolation between two snapshots) and `distance()` (how far apart two states are), which enables undo/redo and parameter morphing.

### 6. Memory and threading

- **PoolAllocator** — fixed-size block allocator with O(1) alloc/free. Used for audio buffers so the graph can hand out temporary buffers without hitting `malloc`.
- **CircularBuffer** — lock-free ring buffer for producer/consumer patterns (e.g., feeding samples from the driver thread to a recorder thread).
- **TaskQueue** — lock-free queue for dispatching work from the audio thread to a background thread (e.g., "save this preset" without blocking process()).
- **WorkerThread / ThreadBarrier / SpinLock** — primitives for coordinating parallel node processing.

---

## Why certain design decisions were made

### TDF-II instead of canonical (Direct Form I or II)

The standard textbook biquad uses four history samples (`x[n-1]`, `x[n-2]`, `y[n-1]`, `y[n-2]`). This works fine in 64-bit floating point but accumulates rounding error at low cutoff frequencies in 32-bit float — the kind of error that turns a 20 Hz high-pass into audible buzzing.

Transposed Direct Form II uses only two state variables per channel (`z1`, `z2`) and computes the output in a single multiply-accumulate chain:

```
y  = b0·x + z1
z1 = b1·x − a1·y + z2
z2 = b2·x − a2·y
```

This structure is numerically superior for audio at float32 precision. The tradeoff is that it's harder to reason about intuitively — the state variables don't correspond to "previous input" or "previous output" the way DF-I does.

### Two-stage coefficient smoothing

When a parameter changes (say, cutoff frequency moves from 1 kHz to 5 kHz), the biquad coefficients must change too. Recomputing coefficients requires `sin()` and `cos()`, which are expensive. Jumping to new coefficients instantly causes a click.

The solution is two layers of smoothing:

1. **Parameter smoothing** (per-block, one-pole filter on the raw frequency/Q/gain values). Advances `numFrames` steps using a pre-cached coefficient. One `sin()`+`cos()` at the end of the block to recompute target coefficients from the smoothed values.

2. **Coefficient smoothing** (per-sample, one-pole on each of b0, b1, b2, a1, a2 toward their targets). No trig math — just multiply-add. Eliminates any remaining step discontinuity between blocks.

### Pimpl everywhere

Every class hides its implementation behind a nested `Impl` class accessed through `unique_ptr<Impl>`. This means:

- Changing internal implementation details (adding a state variable, changing an algorithm) does not change the compiled size or layout of the public class.
- Recompiling an implementation `.cpp` does not force recompilation of every file that includes the header.
- The ABI is stable across builds — important if plugins are loaded at runtime.

### Nyquist clamping in the math, not the UI

`FloatParameter` enforces a `[20, 20000]` range on frequency. But that range is fixed at construction time — it doesn't update when the sample rate changes. A `prepare()` call with a 22050 Hz sample rate makes Nyquist 11025 Hz, and the parameter would happily store 20000.

The clamp lives inside `computeTargetCoefficients()` instead, where both the frequency and the current sample rate are in scope:

```cpp
const float nyquist = sampleRate * 0.5f;
frequency = std::min(frequency, nyquist - 1.0f);
frequency = std::max(frequency, 1.0f);
```

This catches the problem regardless of how the value arrived — direct setter, callback, or smoothed parameter path.
