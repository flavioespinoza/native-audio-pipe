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

## Recent Updates

### ✅ BiQuadFilter Implementation (January 2026)

Complete implementation of high-performance BiQuadFilter with:
- **Transposed Direct Form II topology** for numerical stability
- **Zero heap allocation** in process loop (verified real-time safe)
- **IParameter system integration** with smoothing and automation support
- **Nyquist frequency protection** prevents filter instability at high frequencies
- **Comprehensive documentation** (815 lines API reference + 459 lines architecture)
- **25 passing tests** covering all functionality

**Performance:** ~4.2µs per 512-sample stereo buffer on Apple M1 (0.036% CPU @ 44.1kHz)

See: [docs/api/BiQuadFilter.md](docs/api/BiQuadFilter.md) | [docs/architecture/BiQuadFilter.md](docs/architecture/BiQuadFilter.md)

## To-Do List

### High Priority

#### 🔴 Critical: Fix Driver Compilation Errors
- [ ] **Fix `DeviceInfo` missing type definition**
  - Affects: `AlsaDriver`, `ASIODriver`, `CoreAudioDriver`
  - Files: `src/drivers/*/Driver.cpp`
  - Issue: `error: use of undeclared identifier 'DeviceInfo'`
  - Solution: Add proper type definition or include statement for `DeviceInfo` struct
- [ ] **Fix incorrect `override` keywords on non-virtual functions**
  - Affects: All driver `isRunning()` methods
  - Issue: `error: only virtual member functions can be marked 'override'`
  - Solution: Remove `override` from non-virtual methods or make base class methods virtual
- [ ] **Fix audio callback signature mismatch**
  - Affects: `AlsaDriver::audioCallback`, `ASIODriver::audioCallback`
  - Issue: Expecting `const float*` but receiving `const float**`
  - Solution: Update callback implementation to match `IAudioDriver` interface

#### 🟡 BiQuadFilter Enhancements
- [ ] **Implement remaining filter types**
  - [ ] AllPass filter (phase manipulation)
  - [ ] PeakingEQ (boost/cut at frequency)
  - [ ] LowShelf (boost/cut below cutoff)
  - [ ] HighShelf (boost/cut above cutoff)
  - Coefficient formulas available in Audio EQ Cookbook
- [ ] **Add SIMD optimizations**
  - [ ] AVX/SSE implementation for 4x sample processing
  - [ ] ARM NEON support for Apple Silicon
  - Expected speedup: 2-3x (from 4µs to 1.5µs per buffer)
- [ ] **Add coefficient interpolation for fast modulation**
  - For audio-rate filter sweeps
  - Interpolate coefficients per-sample during modulation

### Medium Priority

#### 📚 Documentation
- [ ] **Generate Doxygen HTML documentation**
  - Set up `docs/Doxyfile` configuration
  - Auto-generate from header comments
  - Host on GitHub Pages
- [ ] **Document remaining audio nodes**
  - Follow BiQuadFilter documentation template
  - API reference + architecture docs for each node
  - Target: 20+ nodes documented
- [ ] **Add DSP theory documentation**
  - `docs/theory/BiQuadFilters.md` - frequency response curves, Z-domain
  - `docs/theory/Nyquist.md` - aliasing, sampling theory
  - `docs/theory/ParameterSmoothing.md` - zipper noise prevention
- [ ] **Create getting started guide**
  - `docs/getting-started.md` with step-by-step tutorial
  - Build your first audio graph
  - Common patterns and best practices

#### 🧪 Testing
- [ ] **Increase test coverage**
  - Current: BiQuadFilter has 25 comprehensive tests
  - Target: 80%+ coverage across all nodes
  - Add tests for graph topology, serialization, threading
- [ ] **Add integration tests**
  - Multi-node graph scenarios
  - Parameter automation workflows
  - Real-time performance under load
- [ ] **Set up continuous benchmarking**
  - Track performance regressions
  - Store benchmark results in repository
  - Generate performance comparison reports

### Low Priority

#### 🚀 Performance
- [ ] **Profile hot paths**
  - Identify bottlenecks in AudioGraph::process()
  - Optimize critical sections
  - Measure memory allocations
- [ ] **Implement optional oversampling for BiQuadFilter**
  - 2x/4x oversampling for high Q settings
  - Reduces aliasing artifacts at extreme resonance
  - Tradeoff: 2-4x CPU cost
- [ ] **Optimize graph execution**
  - Parallelize independent node chains
  - SIMD batch processing for multiple nodes
  - Cache-friendly memory layout

#### 🛠️ Infrastructure
- [ ] **Set up CI/CD pipeline**
  - GitHub Actions for build verification
  - Automated testing on push/PR
  - Multi-platform builds (macOS, Linux, Windows)
- [ ] **Add CMake presets**
  - Development, Release, Debug configurations
  - Platform-specific optimizations
  - Easier onboarding for new contributors
- [ ] **Improve cross-platform support**
  - Fix Windows ASIO driver issues
  - Test Linux ALSA/JACK/PulseAudio drivers
  - Ensure macOS CoreAudio stability

#### 🎨 Features
- [ ] **Add visual graph editor** (future consideration)
  - Node-based UI for graph construction
  - Real-time parameter visualization
  - Waveform/spectrum displays
- [ ] **MIDI integration improvements**
  - MIDI CC to parameter mapping system
  - MIDI learn functionality
  - Note-based modulation sources
- [ ] **Preset browser**
  - UI for loading/saving presets
  - Preset categories and tagging
  - Factory preset library

### Code Quality

- [ ] **Address compiler warnings**
  - Fix all `-Wall -Wextra` warnings
  - Enable `-Werror` for strict builds
- [ ] **Modernize C++ usage**
  - Replace raw pointers with smart pointers where appropriate
  - Use `std::span` for buffer passing (C++20)
  - Adopt `constexpr` for compile-time constants
- [ ] **Add static analysis**
  - clang-tidy integration
  - cppcheck for bug detection
  - Address sanitizer for memory issues

### Documentation Maintenance

- [ ] **Keep benchmark data current**
  - Re-run benchmarks quarterly
  - Update `docs/architecture/BiQuadFilter.md` with latest numbers
  - Flag performance regressions
- [ ] **Review documentation for staleness**
  - Quarterly doc review process
  - Update examples to match current API
  - Archive obsolete documentation

---

## Project Status

| Component | Status | Test Coverage | Documentation |
|-----------|--------|---------------|---------------|
| BiQuadFilter | ✅ Complete | 25/25 tests passing | ✅ Comprehensive |
| Audio Drivers | ⚠️ Compilation errors | - | ⚠️ Needs update |
| Parameter System | ✅ Working | ✅ Good | ⚠️ Needs expansion |
| Audio Graph | ✅ Working | ⚠️ Partial | ⚠️ Needs docs |
| Other Nodes | ⚠️ Various states | ⚠️ Varies | ❌ Missing |

**Legend:**
- ✅ Complete and tested
- ⚠️ Functional but needs work
- ❌ Missing or broken

---

## License

MIT License

## Contributing

1. Fork the repository
2. Create a feature branch
3. Submit a pull request

Issues and feature requests welcome at [GitHub Issues](https://github.com/flavioespinoza/native-audio-pipe/issues).
