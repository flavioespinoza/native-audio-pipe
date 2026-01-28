# native-audio-pipe

A modular, real-time C++ audio engine built around a node graph. Nodes process audio in topologically-sorted order. Every node's parameters are first-class objects with built-in smoothing, range clamping, and change callbacks — so automation, MIDI CC, and preset load all work without touching the audio thread.

## Documentation

| Document | What it covers |
|----------|----------------|
| [Architecture](docs/architecture.md) | How subsystems connect, data flow from driver to output, why TDF-II / why Pimpl / why two-stage smoothing |
| [Adding a node](docs/adding-a-node.md) | Step-by-step walkthrough — header, Pimpl, parameters, tests, benchmark |

Read the architecture doc first. If you want to contribute a new node, the adding-a-node guide walks through every convention the codebase follows.

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cd build && ctest --output-on-failure
```

On ARM macOS, add `-DNAP_ENABLE_SIMD=OFF` to the configure step (the SSE flags are x86-only).

Platform driver stubs for ALSA/JACK/ASIO are gated behind `-DNAP_BUILD_PLATFORM_DRIVERS=ON` and only compile on their respective platforms. Tests and benchmarks build without them.

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
graph.addNode(filter);
graph.addNode(gain);
graph.connect(osc->getNodeId(),    0, filter->getNodeId(), 0);
graph.connect(filter->getNodeId(), 0, gain->getNodeId(),   0);

// Configure
graph.prepare(44100.0, 512);
filter->setFilterType(nap::BiQuadFilter::FilterType::LowPass);
filter->setFrequency(2000.0f);

// Process one block
graph.processBlock(512);
```

## What's in the engine

**Source nodes** — `SineOscillator`, `WhiteNoise`, `PinkNoise`, `ImpulseGenerator`, `FileStreamReader`

**Effect nodes** — `BiQuadFilter` (LP/HP/BP/Notch), `SimpleDelay`, `Chorus`, `Flanger`, `Phaser`, `HardClipper`, `SoftClipper`, `BitCrusher`, `RingModulator`, `ReverbConvolution`

**Math nodes** — `GainNode`, `PanNode`, `MixerNode`, `SplitterNode`, `SummerNode`, `InverterNode`

**Utility nodes** — `MeterNode`, `ScopeNode`, `DCBlockerNode`

**Drivers** — CoreAudio (macOS), ALSA / PulseAudio / JACK (Linux), ASIO (Windows), NullDriver (testing)

**Serialization** — JSON and binary preset save/load with a PresetManager for directory-based preset libraries

**DSP utilities** — FFT, window functions (Hann, Hamming, Blackman, Kaiser, etc.), resampling, DC blocking

## Requirements

- C++17 compiler (GCC 8+, Clang 7+, MSVC 2019+)
- CMake 3.16+
- GoogleTest (fetched automatically at configure time)

## License

MIT
