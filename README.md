# native-audio-pipe

A modular, real-time C++ audio engine built around a node graph. Nodes process audio in topologically-sorted order. Every node's parameters are first-class objects with built-in smoothing, range clamping, and change callbacks — so automation, MIDI CC, and preset load all work without touching the audio thread.

---

## Documentation

| Document | What it covers |
|----------|----------------|
| [Architecture](docs/architecture.md) | How subsystems connect, data flow from driver to output, why TDF-II / why Pimpl / why two-stage smoothing |
| [Adding a node](docs/adding-a-node.md) | Step-by-step walkthrough — header, Pimpl, parameters, tests, benchmark |

Read the architecture doc first. If you want to contribute a new node, the adding-a-node guide walks through every convention the codebase follows.

---

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cd build && ctest --output-on-failure
```

On ARM macOS, add `-DNAP_ENABLE_SIMD=OFF` to the configure step (the SSE flags are x86-only).

Platform driver stubs for ALSA/JACK/ASIO are gated behind `-DNAP_BUILD_PLATFORM_DRIVERS=ON` and only compile on their respective platforms. Tests and benchmarks build without them.

### Build options

| Option | Default | Description |
|--------|---------|-------------|
| `NAP_BUILD_TESTS` | ON | Build unit tests |
| `NAP_BUILD_BENCHMARKS` | ON | Build benchmark library |
| `NAP_ENABLE_SIMD` | ON | Enable SSE4.1 (x86 only) |
| `NAP_BUILD_PLATFORM_DRIVERS` | OFF | Compile ALSA/JACK/ASIO/CoreAudio stubs |

---

## Quick start

```cpp
#include "core/graph/AudioGraph.h"
#include "nodes/source/SineOscillator.h"
#include "nodes/effect/BiQuadFilter.h"
#include "nodes/math/GainNode.h"

nap::AudioGraph graph;

auto osc    = std::make_shared<nap::SineOscillator>();
auto filter = std::make_shared<nap::BiQuadFilter>();
auto gain   = std::make_shared<nap::GainNode>();

// Wire the graph
graph.addNode(osc);
graph.addNode(gain);
graph.addNode(filter);
graph.connect(osc->getNodeId(),    0, filter->getNodeId(), 0);
graph.connect(filter->getNodeId(), 0, gain->getNodeId(),   0);

// Configure
graph.prepare(44100.0, 512);
filter->setFilterType(nap::BiQuadFilter::FilterType::LowPass);
filter->setFrequency(2000.0f);

// Process one block
graph.processBlock(512);
```

---

## What's in the engine

**Source nodes** — `SineOscillator`, `WhiteNoise`, `PinkNoise`, `ImpulseGenerator`, `FileStreamReader`

**Effect nodes** — `BiQuadFilter` (LP/HP/BP/Notch with full IParameter integration, TDF-II, zero-allocation process loop), `SimpleDelay`, `Chorus`, `Flanger`, `Phaser`, `HardClipper`, `SoftClipper`, `BitCrusher`, `RingModulator`, `ReverbConvolution`

**Math nodes** — `GainNode`, `PanNode`, `MixerNode`, `SplitterNode`, `SummerNode`, `InverterNode`

**Utility nodes** — `MeterNode`, `ScopeNode`, `DCBlockerNode`

**Drivers** — CoreAudio (macOS), ALSA / PulseAudio / JACK (Linux), ASIO (Windows), NullDriver (testing)

**Serialization** — JSON and binary preset save/load with a PresetManager for directory-based preset libraries

**DSP utilities** — FFT, window functions (Hann, Hamming, Blackman, Kaiser, etc.), resampling (linear / cubic / windowed-sinc), DC blocking

**Benchmarks** — Per-node micro-benchmarks with volatile DCE guards, warmup phases, and throughput reporting in Msamples/s

---

## Test coverage

407 tests across 55 test suites. Run with:

```bash
cd build && ./nap_tests
```

Filter to a specific suite:

```bash
./nap_tests --gtest_filter="BiQuadFilterTest*"
```

BiQuadFilter tests specifically verify: frequency response (impulse response → single-bin DFT magnitude at passband/stopband), stereo parity (interleaved vs. two independent mono runs), reset behavior, bypass identity, and Nyquist edge cases (frequency above fs/2, sample-rate change that retroactively invalidates an existing frequency).

---

## To-do

Items below are known gaps, planned work, or infrastructure that has not yet been built. Grouped by subsystem.

### CI and cross-platform testing

- [ ] **GitHub Actions matrix** — Linux (`ubuntu-latest`) job with `-DNAP_BUILD_PLATFORM_DRIVERS=ON` to exercise ALSA driver tests. macOS job already covers the driver-excluded path. Windows job for ASIO stubs.
- [ ] **Docker image** — Lightweight Ubuntu image with `libasound2-dev` pre-installed for ad-hoc Linux test runs on ARM Macs without a full VM.
- [ ] **ALSA driver tests on Linux** — `AlsaDriverTest.ConfigureWhileRunningFails` (and any other ALSA tests) are currently skipped on macOS. Needs a Linux runner to actually execute.

### BiQuadFilter — remaining filter types

The `EnumParameter` for `filterType` declares eight modes. Four are implemented (LowPass, HighPass, BandPass, Notch). The remaining four need coefficient formulas wired into `computeTargetCoefficients()`:

- [ ] **AllPass** — phase-shifting filter, unity magnitude at all frequencies
- [ ] **PeakingEQ** — parametric equalizer bell curve, requires gain parameter integration
- [ ] **LowShelf** — boost/cut below a corner frequency
- [ ] **HighShelf** — boost/cut above a corner frequency

All four use the same cookbook formulas (Robert Bristow-Johnson) and the same TDF-II structure. The gain parameter (`paramGain`, already created and smoothed) feeds into the shelf and peaking calculations.

### Resampler

- [ ] **Anti-imaging filter on `upsample()`** — currently inserts zeros between samples with no interpolation filter. Aliased output.
- [ ] **Anti-aliasing filter on `downsample()`** — currently decimates by skipping samples with no low-pass pre-filter. Aliased output.

### Audio graph execution

- [ ] **Block processing with buffer management** — `AudioGraph::processBlock()` has the topology but not the per-node buffer routing that feeds each node's inputs from its upstream outputs.
- [ ] **Topological sort integration** — `rebuildProcessingOrder()` stub exists; needs to call `ExecutionSorter::computeExecutionOrder()`.
- [ ] **Cycle detection on connect** — `FeedbackLoopDetector::wouldCreateCycle()` exists but is not called before `addConnection()`.
- [ ] **Parallel execution** — `ExecutionSorter::computeParallelGroups()` identifies independent node sets at the same graph depth. `WorkerThread` and `TaskQueue` infrastructure exists. Wire them together for multi-threaded graph processing.

### Driver layer

- [ ] **JACK driver** — full stub with ~15 TODOs (server connection, port management, buffer swap, latency query).
- [ ] **ASIO driver** — full stub with ~12 TODOs (driver enumeration, buffer creation, sample-rate negotiation, control panel).
- [ ] **PulseAudio driver** — full stub with ~6 TODOs (stream creation, device enumeration).
- [ ] **ALSA driver** — partial stub; device enumeration and hardware parameter configuration need implementation.
- [ ] **Worker thread priority and CPU affinity** — `WorkerThread` has platform-specific priority/affinity stubs that are no-ops.

### Serialization and file I/O

- [ ] **FileStreamReader** — `load()` and `process()` are stubs. Needs an audio file decoder (WAV at minimum, optional MP3/FLAC via external lib).
- [ ] **ReverbConvolution IR loading** — impulse response file load is a stub.

### Performance and optimization

- [ ] **SIMD paths** — flag and compile options exist (`NAP_ENABLE_SIMD`), but no vectorized inner loops are implemented yet. BiQuadFilter's TDF-II loop is a natural candidate.
- [ ] **MixerNode multi-input** — currently processes a single input stream; needs a summation loop over N connected inputs.

### Documentation

- [ ] **Doxygen on public interfaces** — `IAudioNode.h`, `IParameter.h`, `IAudioDriver.h` deserve doc comments for IDE tooltips and potential future HTML generation. Hold off on a full doxygen build step until the API surface stabilizes.

---

## Requirements

- C++17 compiler (GCC 8+, Clang 7+, MSVC 2019+)
- CMake 3.16+
- GoogleTest (fetched automatically at configure time)

## License

MIT
