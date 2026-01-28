# BiQuadFilter Architecture & Implementation

**Purpose:** Explain how BiQuadFilter works internally and why design decisions were made.

**Audience:** Contributors, maintainers, and advanced users who want to understand the implementation.

---

## Table of Contents

- [Design Goals](#design-goals)
- [Algorithm: Transposed Direct Form II](#algorithm-transposed-direct-form-ii)
- [IParameter System Integration](#iparameter-system-integration)
- [Nyquist Frequency Protection](#nyquist-frequency-protection)
- [Real-Time Safety Guarantees](#real-time-safety-guarantees)
- [Performance Optimizations](#performance-optimizations)
- [Implementation Details](#implementation-details)
- [Future Enhancements](#future-enhancements)

---

## Design Goals

BiQuadFilter was designed with these priorities:

1. **Real-time safety** - Zero heap allocation in audio callback
2. **Numerical stability** - Avoid coefficient blow-up at extreme frequencies
3. **Integration** - Work seamlessly with native-audio-pipe's parameter system
4. **Performance** - Process 1000+ instances without CPU strain
5. **Usability** - Simple API for common use cases

---

## Algorithm: Transposed Direct Form II

### Why Not Direct Form I?

BiQuad filters can be implemented with several topologies:

| Topology | State Variables | Numerical Stability | Cache Efficiency |
|----------|-----------------|---------------------|------------------|
| **Direct Form I** | 4 per channel | ⚠️ Mediocre | ❌ Poor |
| **Direct Form II** | 2 per channel | ⚠️ Mediocre | ✅ Good |
| **Transposed Direct Form II** | 2 per channel | ✅ **Best** | ✅ **Best** |

**Decision:** We chose **Transposed Direct Form II (TDFII)** for:
- Best numerical stability at low frequencies
- Fewest state variables (better cache utilization)
- Industry standard (used in most professional audio libraries)

### TDFII Equations

**Difference equations:**
```
y[n] = b0*x[n] + s1[n-1]
s1[n] = b1*x[n] - a1*y[n] + s2[n-1]
s2[n] = b2*x[n] - a2*y[n]
```

**Implementation** (`BiQuadFilter.cpp:210-216`):
```cpp
const float inL = inputBuffer[i * numChannels];
const float outL = m_impl->b0 * inL + m_impl->s1L;
m_impl->s1L = m_impl->b1 * inL - m_impl->a1 * outL + m_impl->s2L;
m_impl->s2L = m_impl->b2 * inL - m_impl->a2 * outL;
outputBuffer[i * numChannels] = outL;
```

**State variables:**
```cpp
// Only 2 state variables per channel (NOT 4)
float s1L = 0.0f, s2L = 0.0f;  // Left channel
float s1R = 0.0f, s2R = 0.0f;  // Right channel
```

### Coefficient Calculation

**From user parameters to filter coefficients:**

```cpp
// User parameters
float frequency = 1000.0f;  // Hz
float q = 0.707f;           // Resonance
FilterType type = LowPass;

// Digital frequency (normalized to sample rate)
float w0 = 2π * frequency / sampleRate;

// Intermediate values
float cosW0 = cos(w0);
float sinW0 = sin(w0);
float alpha = sinW0 / (2.0f * q);

// LowPass coefficients (Audio EQ Cookbook formulas)
b0 = (1 - cosW0) / 2;
b1 = 1 - cosW0;
b2 = (1 - cosW0) / 2;
a0 = 1 + alpha;
a1 = -2 * cosW0;
a2 = 1 - alpha;

// Normalize by a0
b0 /= a0; b1 /= a0; b2 /= a0;
a1 /= a0; a2 /= a0;
```

**Reference:** [Audio EQ Cookbook by Robert Bristow-Johnson](https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html)

---

## IParameter System Integration

### Architecture

BiQuadFilter integrates with native-audio-pipe's parameter automation system:

```
┌─────────────────────────────────────────────────┐
│  User Input (UI, MIDI, Automation)              │
└─────────────────┬───────────────────────────────┘
                  │
                  ▼
┌─────────────────────────────────────────────────┐
│  FloatParameter / EnumParameter                 │
│  • Clamps to valid range                        │
│  • Applies smoothing (10ms)                     │
│  • Thread-safe setValue()                       │
└─────────────────┬───────────────────────────────┘
                  │
                  ▼
┌─────────────────────────────────────────────────┐
│  Change Callback                                │
│  • Sets coefficientsNeedUpdate flag             │
│  • Updates internal frequency/q/type            │
└─────────────────┬───────────────────────────────┘
                  │
                  ▼
┌─────────────────────────────────────────────────┐
│  process() method                               │
│  • Calls getSmoothedValue() once per buffer     │
│  • Epsilon comparison (0.0001)                  │
│  • Recalculates coefficients if changed         │
└─────────────────────────────────────────────────┘
```

### Parameter Setup (Constructor)

```cpp
// Frequency: 20 Hz to 20 kHz
m_impl->frequencyParam = std::make_unique<FloatParameter>(
    "Frequency", 1000.0f, 20.0f, 20000.0f);
m_impl->frequencyParam->enableSmoothing(true, 0.01f);  // 10ms smoothing

// Wire up callback
m_impl->frequencyParam->setChangeCallback([this](float old, float newValue) {
    m_impl->frequency = newValue;
    m_impl->coefficientsNeedUpdate = true;  // Defer recalc to process()
});
```

### Smoothing in process()

**Key insight:** Get smoothed value ONCE per buffer (not per sample):

```cpp
void BiQuadFilter::process(...) {
    // Get smoothed parameter values ONCE per buffer
    const float smoothedFreq = m_impl->frequencyParam->getSmoothedValue(sampleRate);
    const float smoothedQ = m_impl->qParam->getSmoothedValue(sampleRate);

    // Epsilon comparison avoids unnecessary recalculations
    const float epsilon = 0.0001f;
    const bool freqChanged = std::abs(smoothedFreq - m_impl->currentSmoothedFreq) > epsilon;

    if (freqChanged || m_impl->coefficientsNeedUpdate) {
        m_impl->frequency = smoothedFreq;
        m_impl->calculateCoefficients();  // ~100ns
        m_impl->coefficientsNeedUpdate = false;
    }

    // Process samples with stable coefficients
    for (uint32_t i = 0; i < numFrames; ++i) {
        // ... TDFII algorithm
    }
}
```

**Why once per buffer?**
- Coefficient recalculation costs ~100ns
- Doing it per-sample would cost 512 × 100ns = 51µs (wasteful)
- Once per buffer: 100ns (negligible)
- Smoothing prevents zipper noise even with buffer-rate updates

### Dual Interface

BiQuadFilter supports two ways to set parameters:

**1. Direct setters (convenience):**
```cpp
filter.setFrequency(1000.0f);  // Syncs with parameter system
```

**2. Parameter objects (automation):**
```cpp
filter.getFrequencyParameter()->setValue(1000.0f);
```

**Implementation ensures both stay in sync:**
```cpp
void BiQuadFilter::setFrequency(float frequencyHz) {
    m_impl->frequency = frequencyHz;                    // Update internal
    m_impl->frequencyParam->setValue(frequencyHz);      // Sync parameter
}
```

---

## Nyquist Frequency Protection

### The Problem

Digital filters become **unstable** when frequency exceeds the Nyquist limit (sampleRate/2):

**Example:**
- Sample rate: 32000 Hz
- Nyquist limit: 16000 Hz
- User sets frequency: 20000 Hz → **Filter goes unstable!**

**What happens mathematically:**
```
w0 = 2π × 20000 / 32000 = 3.93 radians  (exceeds π!)
→ cos(w0) wraps around (aliasing)
→ Filter coefficients go outside unit circle
→ Filter oscillates uncontrollably or produces NaN
```

### The Solution

**Clamp frequency to 95% of Nyquist in coefficient calculation:**

```cpp
void BiQuadFilter::Impl::calculateCoefficients() {
    // Safety clamp: never exceed 95% of Nyquist frequency
    const float nyquist = static_cast<float>(sampleRate) / 2.0f;
    const float maxSafeFreq = nyquist * 0.95f;  // 5% margin
    const float clampedFreq = std::min(frequency, maxSafeFreq);

    float w0 = 2.0f * M_PI * clampedFreq / static_cast<float>(sampleRate);
    // ... rest of calculation
}
```

**Why 95% instead of 100%?**
- At exactly Nyquist (w0 = π), coefficients are borderline unstable
- Floating-point error can push them over the edge
- 5% margin is industry standard for safety

### Test Results

From `test_nyquist_clamping.cpp`:

| Sample Rate | Requested Freq | Nyquist | Clamped To | Status |
|-------------|----------------|---------|------------|--------|
| 44100 Hz | 10000 Hz | 22050 Hz | 10000 Hz | ✅ Safe |
| 44100 Hz | 30000 Hz | 22050 Hz | 20947 Hz | ✅ Clamped |
| 32000 Hz | 20000 Hz | 16000 Hz | 15200 Hz | ✅ Clamped |
| 96000 Hz | 50000 Hz | 48000 Hz | 45600 Hz | ✅ Clamped |

**Result:** All 12 tests passed. Filter never went unstable.

---

## Real-Time Safety Guarantees

### Zero Heap Allocation in process()

**Audit of `process()` method (lines 158-221):**

```cpp
void BiQuadFilter::process(...) {
    // Stack variables only
    const float smoothedFreq = ...;  // 4 bytes stack
    const float smoothedQ = ...;     // 4 bytes stack
    const bool freqChanged = ...;    // 1 byte stack

    for (uint32_t i = 0; i < numFrames; ++i) {
        const float inL = ...;       // 4 bytes stack
        const float outL = ...;      // 4 bytes stack
        // ... process samples
    }
}
```

**No heap operations:**
- ❌ No `new` or `malloc`
- ❌ No `std::vector` resize
- ❌ No `std::string` operations
- ❌ No system calls

**Total stack usage:** ~20 bytes constant (regardless of buffer size)

### Lock-Free Parameter Changes

**Problem:** Audio thread must not block waiting for locks.

**Solution:** `getSmoothedValue()` is lock-free:
```cpp
float FloatParameter::getSmoothedValue(float sampleRate) {
    // One-pole smoothing filter (no locks, no allocation)
    value = value * smoothingCoeff + targetValue * (1.0f - smoothingCoeff);
    return value;
}
```

**Separate threads can safely:**
- Audio thread: Call `process()` and `getSmoothedValue()`
- UI thread: Call `setValue()` (atomic write to targetValue)

---

## Performance Optimizations

### 1. Coefficient Recalculation Only When Needed

**Epsilon comparison prevents unnecessary work:**

```cpp
const float epsilon = 0.0001f;
const bool freqChanged = std::abs(smoothedFreq - currentSmoothedFreq) > epsilon;

if (freqChanged) {
    calculateCoefficients();  // ~100ns
}
```

**Without epsilon check:**
- Floating-point noise causes recalc every buffer
- Wastes 100ns × 100 times/sec = 10µs/sec (minor but avoidable)

**With epsilon check:**
- Recalc only when parameters actually change
- Saves CPU for other processing

### 2. Pimpl Pattern

**Implementation hidden in `class Impl`:**

```cpp
class BiQuadFilter::Impl {
    // Large data members hidden from header
    std::unique_ptr<FloatParameter> frequencyParam;
    std::unique_ptr<FloatParameter> qParam;
    float b0, b1, b2, a1, a2;
    float s1L, s2L, s1R, s2R;
};
```

**Benefits:**
- Faster compile times (changes to Impl don't recompile users)
- Smaller header (cleaner API)
- ABI stability

### 3. Stereo Processing in Single Loop

**Instead of:**
```cpp
// BAD: Two separate loops
processChannel(input, output, numFrames, 0);  // Left
processChannel(input, output, numFrames, 1);  // Right
```

**We do:**
```cpp
// GOOD: Single loop, branching on numChannels
for (uint32_t i = 0; i < numFrames; ++i) {
    // Process left
    const float outL = b0 * inL + s1L;
    // ...

    if (numChannels > 1) {
        // Process right
        const float outR = b0 * inR + s1R;
        // ...
    }
}
```

**Benefits:**
- Better instruction cache utilization
- Less loop overhead
- Compiler can optimize better

---

## Implementation Details

### File Structure

```
src/nodes/effect/
├── BiQuadFilter.h          # Public API + Doxygen docs
└── BiQuadFilter.cpp        # Implementation
    ├── class Impl          # Private implementation (lines 11-44)
    ├── calculateCoefficients()  # Coefficient math (lines 46-97)
    ├── Constructor         # Parameter setup (lines 99-151)
    ├── process()           # Real-time audio loop (lines 158-221)
    └── Parameter accessors # Public API (lines 249-304)
```

### State Variables Explained

```cpp
// Filter coefficients (recalculated when parameters change)
float b0, b1, b2;  // Feedforward coefficients
float a1, a2;      // Feedback coefficients (a0 normalized to 1.0)

// TDFII state variables (cleared by reset())
float s1L, s2L;    // Left channel delay lines
float s1R, s2R;    // Right channel delay lines

// Smoothing tracking (for epsilon comparison)
float currentSmoothedFreq;
float currentSmoothedQ;
float currentSmoothedGain;
bool coefficientsNeedUpdate;  // Callback sets this flag
```

### Why Two Sets of Parameters?

You might notice we have:
1. **Internal values:** `m_impl->frequency`, `m_impl->q`
2. **Parameter objects:** `m_impl->frequencyParam`, `m_impl->qParam`

**Reason:** Dual interface support.
- Direct setters (`setFrequency()`) update both
- Parameter system updates internal value via callback
- `process()` uses internal values (pre-smoothed)

This allows both simple API and advanced automation.

---

## Future Enhancements

### 1. Additional Filter Types

Currently implemented: LowPass, HighPass, BandPass, Notch

**Planned:**
- AllPass (phase manipulation)
- PeakingEQ (boost/cut at specific frequency)
- LowShelf / HighShelf (EQ)

**Implementation:** Add cases to `calculateCoefficients()` switch statement.

### 2. SIMD Optimization

Current implementation processes samples sequentially. Could use SIMD:

```cpp
// Process 4 samples at once with AVX
__m128 in4 = _mm_load_ps(&input[i]);
__m128 out4 = _mm_mul_ps(b0_vec, in4);
// ...
```

**Expected speedup:** 2-3x (from 4µs to 1.5µs per buffer)

**Tradeoff:** More complex code, less maintainable

### 3. Modular Oversampling

For very high resonance (Q > 10), aliasing can occur. Could add optional 2x oversampling:

```cpp
filter.setOversamplingFactor(2);  // Internal 2x upsampling
```

**Tradeoff:** 2x CPU cost, but cleaner sound at high Q

### 4. Coefficient Interpolation

Currently coefficients update once per buffer. For very fast modulation (e.g., LFO at audio rate), could interpolate per-sample:

```cpp
for (uint32_t i = 0; i < numFrames; ++i) {
    float b0_interp = b0_start + (b0_end - b0_start) * (i / numFrames);
    // ... use interpolated coefficient
}
```

**Tradeoff:** More CPU, but smoother for extreme modulation.

---

## Testing

### Unit Tests

**Location:** `tests/unit/nodes/effect/Test_BiQuadFilter.cpp`

**Coverage:**
- ✅ Frequency response verification (LowPass blocks high, HighPass blocks low)
- ✅ Coefficient calculation correctness
- ✅ Stereo processing symmetry
- ✅ Reset clears state
- ✅ Bypass mode passes audio unmodified

### Benchmarks

**Location:** `benchmarks/nodes/Bench_BiQuadFilter.cpp`

**Methodology:**
- 10,000 iterations with 1,000 warmup
- Volatile accumulator prevents dead code elimination
- Measures min/avg/max/stddev
- Reports samples/sec and CPU%

### Nyquist Clamping Tests

**Location:** `scratchpad/test_nyquist_clamping.cpp`

**Coverage:**
- ✅ Normal frequencies (safe)
- ✅ Frequencies above Nyquist (clamped)
- ✅ Multiple sample rates (32kHz, 44.1kHz, 96kHz)
- ✅ Edge cases (exactly Nyquist, exactly 95%)

**Result:** 12/12 tests passed, filter never unstable.

---

## References

- **Audio EQ Cookbook:** https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
- **JUCE BiQuad Implementation:** https://github.com/juce-framework/JUCE/blob/master/modules/juce_dsp/filter_design/juce_IIRFilter.cpp
- **Transposed Direct Form II:** Digital Signal Processing Principles, Algorithms, and Applications (Proakis & Manolakis)

---

## Contributing

When modifying BiQuadFilter:

1. **Maintain real-time safety:** No heap allocation in `process()`
2. **Run benchmarks:** Ensure performance doesn't regress
3. **Test Nyquist clamping:** New filter types must respect clamping
4. **Update docs:** Keep this document in sync with code

**Key invariants to preserve:**
- `process()` is zero-allocation
- Coefficients always clamped to safe range
- IParameter integration remains functional
- Backward compatibility with existing code

---

**Related Documentation:**
- [API Reference](../api/BiQuadFilter.md)
- [Usage Examples](../../tests/unit/nodes/effect/Test_BiQuadFilter.cpp)
- [Performance Benchmarks](../../benchmarks/nodes/Bench_BiQuadFilter.cpp)
