# BiQuadFilter API Reference

**Class:** `nap::BiQuadFilter`
**Header:** `src/nodes/effect/BiQuadFilter.h`
**Inherits:** `IAudioNode`

---

## Table of Contents

- [Overview](#overview)
- [Quick Start](#quick-start)
- [Filter Types](#filter-types)
- [Core Methods](#core-methods)
- [Parameter Control](#parameter-control)
- [Automation Integration](#automation-integration)
- [Performance Characteristics](#performance-characteristics)
- [Common Patterns](#common-patterns)
- [Troubleshooting](#troubleshooting)

---

## Overview

BiQuadFilter is a high-performance second-order IIR filter implementing multiple filter topologies. It uses **Transposed Direct Form II** for numerical stability and is designed for real-time audio processing with zero heap allocation in the process loop.

**Key Features:**
- ✅ Real-time safe (zero allocation in `process()`)
- ✅ Four filter types: LowPass, HighPass, BandPass, Notch
- ✅ Integrated with IParameter system for automation
- ✅ Automatic Nyquist frequency protection
- ✅ Parameter smoothing (10ms) prevents zipper noise
- ✅ ~4µs per 512-sample stereo buffer

---

## Quick Start

### Basic Usage

```cpp
#include "nodes/effect/BiQuadFilter.h"

// Create filter
nap::BiQuadFilter lpf;

// Initialize (call once before audio starts)
lpf.prepare(44100.0, 512);

// Configure filter
lpf.setFilterType(nap::BiQuadFilter::FilterType::LowPass);
lpf.setFrequency(1000.0f);  // 1kHz cutoff
lpf.setQ(0.707f);           // Butterworth response

// In audio callback (real-time safe)
lpf.process(inputBuffer, outputBuffer, numFrames, 2);
```

### In-Place Processing

```cpp
// Output overwrites input (saves memory)
lpf.process(buffer, buffer, numFrames, numChannels);
```

---

## Filter Types

### LowPass
Passes frequencies below cutoff, attenuates above.

```cpp
filter.setFilterType(BiQuadFilter::FilterType::LowPass);
filter.setFrequency(1000.0f);  // Pass 20Hz-1kHz, attenuate above
filter.setQ(0.707f);           // Butterworth (flat passband)
```

**Use cases:**
- Removing high-frequency noise
- Bass-only mixing (subwoofer crossover)
- Anti-aliasing before downsampling
- Simulating "muffled" sound effects

**Frequency response:**
- At cutoff: ~-3dB (70.7% amplitude)
- One octave above: ~-12dB (25% amplitude)
- Two octaves above: ~-24dB (6% amplitude)

### HighPass
Passes frequencies above cutoff, attenuates below.

```cpp
filter.setFilterType(BiQuadFilter::FilterType::HighPass);
filter.setFrequency(100.0f);   // Remove rumble below 100Hz
filter.setQ(0.707f);
```

**Use cases:**
- Removing low-frequency rumble
- Cutting DC offset
- Vocal processing (remove proximity effect)
- Treble-only mixing

### BandPass
Passes frequencies near cutoff, attenuates others.

```cpp
filter.setFilterType(BiQuadFilter::FilterType::BandPass);
filter.setFrequency(1000.0f);  // Center at 1kHz
filter.setQ(5.0f);             // Narrow band (higher Q = narrower)
```

**Use cases:**
- Isolating specific frequency ranges
- Telephone/radio effect
- Resonant sweeps (filter modulation)
- Removing everything except vocals (~300-3000Hz)

**Q factor effect:**
- Q = 0.707: Wide band (±1 octave)
- Q = 5.0: Narrow band (±1/5 octave)
- Q = 10.0: Very narrow (almost a tone)

### Notch (Band-Reject)
Attenuates frequencies near cutoff, passes others.

```cpp
filter.setFilterType(BiQuadFilter::FilterType::Notch);
filter.setFrequency(60.0f);    // Remove 60Hz hum
filter.setQ(5.0f);             // Narrow notch
```

**Use cases:**
- Removing 50Hz/60Hz power line hum
- Eliminating feedback frequencies
- Canceling specific tones

---

## Core Methods

### `prepare(double sampleRate, uint32_t blockSize)`

Initialize filter with sample rate. **Must call before `process()`.**

```cpp
filter.prepare(44100.0, 512);
```

**When to call:**
- Application startup
- Sample rate changes
- After loading a new project

**Thread safety:** ❌ NOT real-time safe (call before audio starts)

---

### `process(const float* in, float* out, uint32_t frames, uint32_t channels)`

Process audio through filter. **Real-time safe.**

```cpp
// Stereo processing
filter.process(inputBuffer, outputBuffer, 512, 2);

// Mono processing
filter.process(monoInput, monoOutput, 512, 1);

// In-place (input == output)
filter.process(buffer, buffer, 512, 2);
```

**Parameters:**
- `in`: Interleaved input (e.g., L,R,L,R for stereo)
- `out`: Interleaved output (can be same as input)
- `frames`: Number of frames (samples per channel)
- `channels`: 1 (mono) or 2 (stereo)

**Performance:** ~4µs for 512 stereo frames on Apple M1

**Thread safety:** ❌ Call only from audio thread

---

### `reset()`

Clear filter state (delay lines). Call after silence or seeks.

```cpp
filter.reset();  // Clear filter memory
```

**When to call:**
- Starting playback after pause
- After timeline seek
- When switching between audio sources
- If you hear clicks/pops from previous audio

**Thread safety:** ✅ Real-time safe

---

## Parameter Control

### Direct Control (Simple)

Use `set*()` methods for immediate control:

```cpp
filter.setFrequency(2000.0f);  // Change cutoff
filter.setQ(1.5f);             // Change resonance
filter.setFilterType(BiQuadFilter::FilterType::HighPass);
```

**Best for:**
- Manual control
- One-time initialization
- Simple applications without automation

**Smoothing:** Built-in (10ms) prevents zipper noise

---

### Parameter Objects (Advanced)

Use `get*Parameter()` for automation, presets, and UI binding:

```cpp
// Get parameter object
FloatParameter* freqParam = filter.getFrequencyParameter();

// Set value (smoothed automatically)
freqParam->setValue(automationValue);

// Get range
float min = freqParam->getMin();  // 20.0
float max = freqParam->getMax();  // 20000.0

// Check current value
float current = freqParam->getValue();
```

**Best for:**
- DAW automation
- UI sliders
- MIDI CC mapping
- Preset systems

---

## Automation Integration

### Connecting to UI Slider

```cpp
class FilterUI {
    BiQuadFilter filter;

    void onSliderChange(float normalizedValue) {
        // Convert 0-1 to 20-20000 Hz (logarithmic)
        float freq = 20.0f * std::pow(1000.0f, normalizedValue);

        filter.getFrequencyParameter()->setValue(freq);
        // Automatically smoothed over 10ms
    }
};
```

### MIDI CC Control

```cpp
void onMidiCC(int cc, int value) {
    if (cc == 74) {  // Filter cutoff
        float normalized = value / 127.0f;
        float freq = 20.0f * std::pow(1000.0f, normalized);
        filter.getFrequencyParameter()->setValue(freq);
    }
}
```

### DAW Automation

```cpp
void processBlock(AudioBuffer& buffer, AutomationData& automation) {
    // Read automation envelope
    for (int i = 0; i < numSamples; ++i) {
        float automatedFreq = automation.getFrequency(i);
        filter.getFrequencyParameter()->setValue(automatedFreq);
    }

    filter.process(buffer.getData(), buffer.getData(),
                   buffer.getNumSamples(), buffer.getNumChannels());
}
```

---

## Performance Characteristics

### Benchmarks (Apple M1, clang++ -O3)

| Configuration      | Time/Buffer | Throughput     | CPU @ 44.1kHz |
|--------------------|-------------|----------------|---------------|
| LowPass (stereo)   | 4.2 µs      | 121 MSamples/s | 0.036%        |
| HighPass (stereo)  | 4.0 µs      | 128 MSamples/s | 0.034%        |
| BandPass (stereo)  | 4.0 µs      | 128 MSamples/s | 0.034%        |

**Buffer size:** 512 samples per channel
**Real-time headroom:** ~2750x (can run 2000+ instances)

### Memory Footprint

- **Per instance:** ~200 bytes (Impl object + parameters)
- **Stack per process() call:** ~16 bytes
- **Heap allocation in process():** ✅ **ZERO**

### Coefficient Recalculation Cost

Filter coefficients are recalculated only when parameters change:

```cpp
// Coefficient recalc: ~100ns (negligible)
filter.setFrequency(2000.0f);  // Triggers recalc on next process()
```

Recalculation uses **epsilon comparison (0.0001)** to avoid unnecessary updates from floating-point noise.

---

## Common Patterns

### Resonant Sweep Effect

```cpp
void updateSweep(BiQuadFilter& filter, float time) {
    // Sweep from 200Hz to 5kHz over 4 seconds
    float t = fmod(time, 4.0f) / 4.0f;  // 0.0 to 1.0
    float freq = 200.0f + t * 4800.0f;

    filter.setFilterType(BiQuadFilter::FilterType::BandPass);
    filter.setFrequency(freq);
    filter.setQ(8.0f);  // High resonance
}
```

### Subwoofer Crossover (LowPass + HighPass)

```cpp
BiQuadFilter lowpass, highpass;

// Initialize
lowpass.prepare(44100.0, 512);
highpass.prepare(44100.0, 512);

// Configure crossover at 80Hz
lowpass.setFilterType(BiQuadFilter::FilterType::LowPass);
lowpass.setFrequency(80.0f);
lowpass.setQ(0.707f);

highpass.setFilterType(BiQuadFilter::FilterType::HighPass);
highpass.setFrequency(80.0f);
highpass.setQ(0.707f);

// Process
lowpass.process(input, subwooferOut, 512, 2);  // Bass to subwoofer
highpass.process(input, mainOut, 512, 2);      // Mids/highs to main speakers
```

### Telephone Effect

```cpp
filter.setFilterType(BiQuadFilter::FilterType::BandPass);
filter.setFrequency(1500.0f);  // Center of speech range
filter.setQ(1.5f);             // Moderate bandwidth
```

### Remove 60Hz Hum

```cpp
filter.setFilterType(BiQuadFilter::FilterType::Notch);
filter.setFrequency(60.0f);   // US power line frequency (use 50Hz for EU)
filter.setQ(10.0f);           // Very narrow notch
```

---

## Troubleshooting

### Problem: No audible effect

**Possible causes:**
1. Filter bypassed: `filter.isBypassed() == true`
2. Frequency beyond audible range
3. Q too low (gentle slope)

**Solution:**
```cpp
filter.setBypassed(false);
filter.setFrequency(1000.0f);  // Middle of audible range
filter.setQ(0.707f);           // Standard response
```

---

### Problem: Clicks/pops when changing parameters

**Cause:** Abrupt parameter changes without smoothing.

**Solution:** Use IParameter system (smoothing enabled by default):
```cpp
// GOOD: Smoothed automatically
filter.getFrequencyParameter()->setValue(newFreq);

// ALSO GOOD: Direct setters have smoothing too
filter.setFrequency(newFreq);
```

---

### Problem: Filter sounds "wrong" at different sample rates

**Cause:** Forgot to call `prepare()` after sample rate change.

**Solution:**
```cpp
// When sample rate changes:
filter.prepare(newSampleRate, blockSize);
```

---

### Problem: Artifacts when frequency exceeds Nyquist

**Solution:** ✅ **Automatically handled!** Filter clamps to 95% of Nyquist.

Example at 32kHz sample rate:
```cpp
filter.prepare(32000.0, 512);      // Nyquist = 16kHz
filter.setFrequency(20000.0f);     // Request 20kHz
// Actual frequency used: 15200Hz (95% of 16kHz)
```

---

### Problem: Filter unstable / producing NaN

**Should never happen.** If it does:

1. Check sample rate is valid: `sampleRate > 0`
2. Check Q is reasonable: `0.1 <= Q <= 20.0`
3. Call `filter.reset()` to clear state
4. File a bug report with reproduction steps

---

## See Also

- **Implementation details:** [docs/architecture/BiQuadFilter.md](../architecture/BiQuadFilter.md)
- **DSP theory:** [docs/theory/BiQuadFilters.md](../theory/BiQuadFilters.md) *(coming soon)*
- **Example code:** [tests/unit/nodes/effect/Test_BiQuadFilter.cpp](../../tests/unit/nodes/effect/Test_BiQuadFilter.cpp)
- **Benchmarks:** [benchmarks/nodes/Bench_BiQuadFilter.cpp](../../benchmarks/nodes/Bench_BiQuadFilter.cpp)

---

**Next:** Read [Architecture Documentation](../architecture/BiQuadFilter.md) to understand how BiQuadFilter works internally.
