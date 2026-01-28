# native-audio-pipe

Enterprise-grade C++ audio engine with modular node-based architecture.

## Features

- **Modular Node Graph** - Flexible audio routing with automatic execution ordering
- **24 Audio Nodes** - Oscillators, filters, effects, and utilities
- **Real-time Safe** - Lock-free data structures and pool allocators
- **Cross-Platform Drivers** - CoreAudio (macOS), ALSA/PulseAudio/JACK (Linux), ASIO (Windows)
- **Parameter System** - Type-safe parameters with smoothing and automation
- **Serialization** - JSON and binary preset save/load
- **DSP Utilities** - FFT, window functions, resampling, DC blocking

## Project Structure

```
native-audio-pipe/
├── src/
│   ├── api/                    # Public interfaces
│   │   ├── IAudioNode.h
│   │   ├── IAudioSource.h
│   │   ├── IAudioSink.h
│   │   └── IParameter.h
│   ├── core/
│   │   ├── graph/              # Audio graph management
│   │   ├── memory/             # Lock-free buffers, allocators
│   │   ├── threading/          # Worker threads, task queues
│   │   ├── parameters/         # Typed parameter system
│   │   └── serialization/      # JSON/Binary serializers
│   ├── nodes/
│   │   ├── math/               # Gain, Pan, Mixer, Splitter, etc.
│   │   ├── source/             # Oscillators, Noise, File reader
│   │   ├── effect/             # Filters, Delays, Distortion, Reverb
│   │   └── utility/            # Meters, Scopes, DC Blocker
│   ├── drivers/                # Platform audio drivers
│   └── utils/dsp/              # FFT, Windows, Resampler
├── tests/unit/                 # GoogleTest unit tests
├── docs/nodes/                 # Node documentation
└── cmake/                      # CMake modules
```

## Requirements

- CMake 3.16+
- C++17 compiler (GCC 8+, Clang 7+, MSVC 2019+)
- GoogleTest (auto-fetched if not found)

### Platform-specific

- **macOS**: Xcode Command Line Tools
- **Linux**: ALSA dev libraries (`libasound2-dev`)
- **Windows**: Visual Studio 2019+

## Build

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --parallel

# Run tests
cd build && ctest --output-on-failure
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `NAP_BUILD_TESTS` | ON | Build unit tests |
| `NAP_BUILD_EXAMPLES` | OFF | Build example applications |
| `NAP_ENABLE_SIMD` | ON | Enable SSE4.1 optimizations |
| `NAP_USE_SYSTEM_GTEST` | OFF | Use system GoogleTest |

## Quick Start

### Basic Audio Processing

```cpp
#include "core/graph/AudioGraph.h"
#include "nodes/source/SineOscillator.h"
#include "nodes/math/GainNode.h"

// Create nodes
auto osc = nap::SineOscillatorFactory::create(440.0f);
auto gain = nap::GainNodeFactory::create(0.5f);

// Build graph
nap::AudioGraph graph;
graph.addNode(osc);
graph.addNode(gain);
graph.connect(osc->getOutputPort(0), gain->getInputPort(0));

// Process audio
graph.process(buffer, numFrames);
```

### BiQuadFilter Example

```cpp
#include "nodes/effect/BiQuadFilter.h"

// Create and configure filter
nap::BiQuadFilter lpf;
lpf.prepare(44100.0, 512);
lpf.setFilterType(nap::BiQuadFilter::FilterType::LowPass);
lpf.setFrequency(1000.0f);  // 1kHz cutoff
lpf.setQ(0.707f);           // Butterworth response

// Real-time processing (zero allocation)
lpf.process(inputBuffer, outputBuffer, numFrames, 2);

// Parameter automation
auto* freqParam = lpf.getFrequencyParameter();
freqParam->setValue(2000.0f);  // Smoothed over 10ms
```

**[→ Complete BiQuadFilter API Reference](docs/api/BiQuadFilter.md)**

## Audio Nodes

### Source Nodes
- `SineOscillator` - Sine wave generator
- `WhiteNoise` - White noise generator
- `PinkNoise` - Pink noise (1/f) generator
- `ImpulseGenerator` - Impulse/click generator
- `FileStreamReader` - Audio file playback

### Math Nodes
- `GainNode` - Volume control
- `PanNode` - Stereo panning
- `MixerNode` - Multi-input mixer
- `SplitterNode` - Signal splitter
- `SummerNode` - Signal summer
- `InverterNode` - Phase inversion

### Effect Nodes
- **`BiQuadFilter`** - High-performance biquad filter (LowPass, HighPass, BandPass, Notch)
  - Transposed Direct Form II topology
  - Zero allocation in process loop (~4µs per 512 samples)
  - [API Reference](docs/api/BiQuadFilter.md) | [Architecture](docs/architecture/BiQuadFilter.md)
- `SimpleDelay` - Basic delay line
- `Chorus` - Chorus effect
- `Flanger` - Flanger effect
- `Phaser` - Phaser effect
- `HardClipper` - Hard clipping distortion
- `SoftClipper` - Soft saturation
- `BitCrusher` - Bit depth reduction
- `RingModulator` - Ring modulation
- `ReverbConvolution` - Convolution reverb

### Utility Nodes
- `MeterNode` - Level metering
- `ScopeNode` - Waveform display
- `DCBlockerNode` - DC offset removal

## Documentation

- **[BiQuadFilter API](docs/api/BiQuadFilter.md)** - Complete usage guide with examples
- **[BiQuadFilter Architecture](docs/architecture/BiQuadFilter.md)** - Implementation details and design decisions
- **Node Documentation** - Coming soon (see `docs/api/` for available nodes)

## License

MIT License

## Contributing

1. Fork the repository
2. Create a feature branch
3. Submit a pull request

Issues and feature requests welcome at [GitHub Issues](https://github.com/flavioespinoza/native-audio-pipe/issues).
