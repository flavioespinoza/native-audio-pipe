# Adding a new audio node

This guide walks you through creating a new effect node from scratch, using `BiQuadFilter` as the reference implementation. Every step here mirrors what BiQuadFilter does — follow the same pattern and you will not go wrong.

---

## Step 1 — The header

Create your header in `src/nodes/effect/` (or the appropriate category directory). Declare the public interface and a Pimpl pointer:

```cpp
// src/nodes/effect/MyEffect.h
#ifndef NAP_MY_EFFECT_H
#define NAP_MY_EFFECT_H

#include "../../api/IAudioNode.h"
#include <string>
#include <cstdint>
#include <memory>

namespace nap {

class MyEffect final : public IAudioNode {
public:
    MyEffect();
    ~MyEffect();

    // Move semantics — no copies (unique_ptr member)
    MyEffect(MyEffect&&) noexcept;
    MyEffect& operator=(MyEffect&&) noexcept;

    // --- IAudioNode contract ---
    void process(const float* inputBuffer, float* outputBuffer,
                 std::uint32_t numFrames, std::uint32_t numChannels) override;
    void prepare(double sampleRate, std::uint32_t blockSize) override;
    void reset() override;

    std::string getNodeId() const override;
    std::string getTypeName() const override;
    std::uint32_t getNumInputChannels() const override;
    std::uint32_t getNumOutputChannels() const override;
    bool isBypassed() const override;
    void setBypassed(bool bypassed) override;

    // --- Your public API ---
    void setDryWet(float mix);        // 0.0 = dry, 1.0 = wet
    float getDryWet() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif
```

Key points:
- `final` — nodes are not meant to be subclassed further.
- Move-only — the Pimpl `unique_ptr` makes copies meaningless.
- Public API is small. Internal state lives in `Impl`.

---

## Step 2 — The implementation

Create the `.cpp` in the same directory. The `Impl` class holds everything:

```cpp
// src/nodes/effect/MyEffect.cpp
#include "MyEffect.h"
#include "../../core/parameters/FloatParameter.h"
#include <algorithm>
#include <cmath>

namespace nap {

class MyEffect::Impl {
public:
    std::string nodeId;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;

    // IParameter-backed controls
    std::unique_ptr<FloatParameter> paramDryWet;

    // Internal state (delay lines, accumulators, etc.)
    // ...

    // Cached values recomputed in prepare()
    // ...
};
```

### Constructor — create parameters, wire callbacks

```cpp
MyEffect::MyEffect()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "MyEffect_" + std::to_string(++instanceCounter);

    // Create parameter with name, default, min, max
    m_impl->paramDryWet = std::make_unique<FloatParameter>(
        "dryWet", 0.5f, 0.0f, 1.0f
    );
    m_impl->paramDryWet->enableSmoothing(true, 0.010f);  // 10 ms

    // Wire change callback — fires on any external write
    auto* impl = m_impl.get();
    m_impl->paramDryWet->setChangeCallback(
        [impl](float, float) { /* recompute any derived state */ }
    );
}
```

Why the callback captures a raw pointer to `Impl` instead of `this` or a `shared_ptr`: the node owns both the `Impl` and the parameters. The callback's lifetime is bounded by the parameter's lifetime, which is bounded by the node. A raw pointer is correct and avoids reference-count overhead.

### prepare() — cache sample-rate-dependent state

```cpp
void MyEffect::prepare(double sampleRate, std::uint32_t blockSize) {
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;

    // Recompute anything that depends on sample rate
    // (smoothing coefficients, delay lengths, etc.)

    // Snap smoothed values to current targets — no ramp on rate change
    // ...
}
```

### process() — the hot path

```cpp
void MyEffect::process(const float* inputBuffer, float* outputBuffer,
                        std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    // Read parameter targets once per block
    const float targetDryWet = m_impl->paramDryWet->getValue();

    // Local copies of state — avoid repeated pointer chasing in the loop
    float smoothedDryWet = m_impl->smoothedDryWet;
    const float psc = m_impl->paramSmoothCoeff;

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        // Advance parameter smoother
        smoothedDryWet = targetDryWet + psc * (smoothedDryWet - targetDryWet);

        // Your DSP here — all stack locals, no allocation
        float x = inputBuffer[i * numChannels];
        float wet = /* ... your effect math ... */;
        float y = smoothedDryWet * wet + (1.0f - smoothedDryWet) * x;
        outputBuffer[i * numChannels] = y;
    }

    // Write state back at end of block
    m_impl->smoothedDryWet = smoothedDryWet;
}
```

The pattern: load state into locals before the loop, do all work with those locals, write back after the loop. This gives the compiler maximum freedom to keep values in registers.

### reset() — clear state

```cpp
void MyEffect::reset() {
    // Zero out all delay lines, accumulators, filter state
    // Snap smoothed values to current targets (no ramp after reset)
}
```

### Boilerplate getters

```cpp
std::string MyEffect::getNodeId() const { return m_impl->nodeId; }
std::string MyEffect::getTypeName() const { return "MyEffect"; }
std::uint32_t MyEffect::getNumInputChannels() const { return 2; }
std::uint32_t MyEffect::getNumOutputChannels() const { return 2; }
bool MyEffect::isBypassed() const { return m_impl->bypassed; }
void MyEffect::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void MyEffect::setDryWet(float mix) { m_impl->paramDryWet->setValue(mix); }
float MyEffect::getDryWet() const { return m_impl->paramDryWet->getValue(); }
```

Setters delegate entirely to the parameter object. The parameter handles clamping and fires the change callback — the setter does nothing else.

---

## Step 3 — Wire into CMakeLists.txt

Add your source file to the `NAP_NODE_SOURCES` list:

```cmake
set(NAP_NODE_SOURCES
    # ...
    src/nodes/effect/MyEffect.cpp
    # ...
)
```

That is all. The node is automatically linked into the library and available to tests and benchmarks.

---

## Step 4 — Write tests

Create `tests/unit/nodes/effect/Test_MyEffect.cpp`. The test file is picked up automatically by the `GLOB_RECURSE` in CMakeLists.

### Basic interface tests

```cpp
#include <gtest/gtest.h>
#include "../../../../src/nodes/effect/MyEffect.h"

TEST(MyEffectTest, HasCorrectTypeName) {
    MyEffect effect;
    EXPECT_EQ(effect.getTypeName(), "MyEffect");
}

TEST(MyEffectTest, DefaultDryWetIs0_5) {
    MyEffect effect;
    EXPECT_FLOAT_EQ(effect.getDryWet(), 0.5f);
}
```

### Frequency response / functional tests

Use impulse response analysis to verify your effect actually does what it claims:

```cpp
static std::vector<float> captureImpulseResponse(MyEffect& effect, uint32_t numSamples)
{
    std::vector<float> input(numSamples, 0.0f);
    std::vector<float> output(numSamples, 0.0f);
    input[0] = 1.0f;  // unit impulse

    effect.reset();
    effect.process(input.data(), output.data(), numSamples, 1);
    return output;
}
```

Then evaluate the impulse response at specific frequencies using a single-bin DFT. Check that passband is near 0 dB and stopband is sufficiently attenuated. See `Test_BiQuadFilter.cpp` for the full helper functions (`magnitudeAtFrequency`, `magnitudeToDb`).

### Edge-case tests

Always test:
- **Reset clears state** — process some signal, reset, process zeros, verify output is zero.
- **Stereo parity** — process two mono signals independently, then as an interleaved stereo pair. Results must match sample-for-sample.
- **Bypass is identity** — when bypassed, output equals input exactly.
- **Parameter boundaries** — set parameters to min, max, and values beyond range. Verify no NaN, no crash, sensible behavior.

---

## Step 5 — Write a benchmark

Create `benchmarks/nodes/Bench_MyEffect.cpp` and `benchmarks/nodes/Bench_MyEffect.h`.

The benchmark structure follows the same pattern as `Bench_BiQuadFilter`:

```cpp
// The DCE guard — without this, the compiler legally optimizes
// away the entire process() loop at -O2/-O3, producing fake
// sub-nanosecond timings.
static volatile float g_dce_sink = 0.0f;

// In the timed loop:
for (uint64_t i = 0; i < iterations; ++i) {
    effect.reset();

    auto start = std::chrono::high_resolution_clock::now();
    effect.process(inputBuffer.data(), outputBuffer.data(),
                   static_cast<uint32_t>(bufferSize), 1);
    auto end = std::chrono::high_resolution_clock::now();

    // This is the DCE guard. The compiler must keep the loop alive
    // because the last output sample escapes to a volatile location.
    g_dce_sink += outputBuffer[bufferSize - 1];

    times.push_back(std::chrono::duration<double, std::nano>(end - start).count());
}
```

Why `volatile`? The compiler can prove that nothing *reads* `outputBuffer` after `process()` returns. Without a side effect that escapes the translation unit, it can delete the loop entirely. `volatile` on the sink variable tells the compiler "this write is observable — you cannot assume it is dead." The `+=` (not `=`) forces the compiler to actually read the last sample, not just write a constant.

Add your benchmark source to `NAP_BENCHMARK_SOURCES` in CMakeLists.txt.

---

## Checklist

Before submitting a new node:

- [ ] Header in the correct category directory with Pimpl pattern
- [ ] All parameters created as `FloatParameter`/`EnumParameter`/etc. with explicit range
- [ ] Smoothing enabled on continuous parameters (`enableSmoothing(true, 0.010f)`)
- [ ] Change callbacks wired in constructor
- [ ] `process()` uses only stack locals in the hot loop — no allocation, no blocking
- [ ] `prepare()` recomputes all sample-rate-dependent cached values
- [ ] `reset()` clears all delay/filter state and snaps smoothed values to targets
- [ ] Bypass short-circuits to `std::copy`
- [ ] Setters delegate entirely to parameter `setValue()` — no manual callback calls
- [ ] CMakeLists.txt updated
- [ ] Unit tests cover: interface, functional behavior, reset, stereo parity, bypass, edge cases
- [ ] Benchmark with volatile DCE guard
