#include "utils/dsp/DCBlocker.h"
#include <cmath>
#include <vector>

namespace nap {

// ==================== DCBlocker Implementation ====================

class DCBlocker::Impl {
public:
    float R = 0.995f;   // Pole radius
    float x1 = 0.0f;    // Previous input
    float y1 = 0.0f;    // Previous output

    Impl() = default;
    explicit Impl(float r) : R(r) {}
};

DCBlocker::DCBlocker()
    : pImpl(std::make_unique<Impl>()) {}

DCBlocker::DCBlocker(float R)
    : pImpl(std::make_unique<Impl>(R)) {}

DCBlocker::~DCBlocker() = default;

DCBlocker::DCBlocker(DCBlocker&&) noexcept = default;
DCBlocker& DCBlocker::operator=(DCBlocker&&) noexcept = default;

void DCBlocker::setR(float R) {
    pImpl->R = R;
}

float DCBlocker::getR() const {
    return pImpl->R;
}

void DCBlocker::setCutoffFrequency(float frequency, float sampleRate) {
    pImpl->R = calculateR(frequency, sampleRate);
}

float DCBlocker::getCutoffFrequency(float sampleRate) const {
    // Approximate inverse: fc = fs * (1 - R) / (2 * pi)
    return sampleRate * (1.0f - pImpl->R) / (2.0f * static_cast<float>(M_PI));
}

void DCBlocker::reset() {
    pImpl->x1 = 0.0f;
    pImpl->y1 = 0.0f;
}

float DCBlocker::process(float input) {
    // y[n] = x[n] - x[n-1] + R * y[n-1]
    float output = input - pImpl->x1 + pImpl->R * pImpl->y1;
    pImpl->x1 = input;
    pImpl->y1 = output;
    return output;
}

void DCBlocker::process(float* buffer, size_t numSamples) {
    for (size_t i = 0; i < numSamples; ++i) {
        buffer[i] = process(buffer[i]);
    }
}

void DCBlocker::process(const float* input, float* output, size_t numSamples) {
    for (size_t i = 0; i < numSamples; ++i) {
        output[i] = process(input[i]);
    }
}

float DCBlocker::calculateR(float cutoffFrequency, float sampleRate) {
    // R = 1 - (2 * pi * fc / fs)
    // Clamped to valid range
    float R = 1.0f - (2.0f * static_cast<float>(M_PI) * cutoffFrequency / sampleRate);
    return std::max(0.9f, std::min(0.9999f, R));
}

// ==================== MultiChannelDCBlocker Implementation ====================

class MultiChannelDCBlocker::Impl {
public:
    std::vector<DCBlocker> blockers;
    size_t numChannels;
    float R;

    Impl(size_t channels, float r) : numChannels(channels), R(r) {
        blockers.reserve(channels);
        for (size_t i = 0; i < channels; ++i) {
            blockers.emplace_back(r);
        }
    }
};

MultiChannelDCBlocker::MultiChannelDCBlocker(size_t numChannels, float R)
    : pImpl(std::make_unique<Impl>(numChannels, R)) {}

MultiChannelDCBlocker::~MultiChannelDCBlocker() = default;

MultiChannelDCBlocker::MultiChannelDCBlocker(MultiChannelDCBlocker&&) noexcept = default;
MultiChannelDCBlocker& MultiChannelDCBlocker::operator=(MultiChannelDCBlocker&&) noexcept = default;

size_t MultiChannelDCBlocker::getNumChannels() const {
    return pImpl->numChannels;
}

void MultiChannelDCBlocker::setNumChannels(size_t numChannels) {
    if (numChannels != pImpl->numChannels) {
        pImpl->numChannels = numChannels;
        pImpl->blockers.clear();
        pImpl->blockers.reserve(numChannels);
        for (size_t i = 0; i < numChannels; ++i) {
            pImpl->blockers.emplace_back(pImpl->R);
        }
    }
}

void MultiChannelDCBlocker::setR(float R) {
    pImpl->R = R;
    for (auto& blocker : pImpl->blockers) {
        blocker.setR(R);
    }
}

void MultiChannelDCBlocker::setCutoffFrequency(float frequency, float sampleRate) {
    float R = DCBlocker::calculateR(frequency, sampleRate);
    setR(R);
}

void MultiChannelDCBlocker::reset() {
    for (auto& blocker : pImpl->blockers) {
        blocker.reset();
    }
}

void MultiChannelDCBlocker::processInterleaved(float* buffer, size_t numFrames) {
    for (size_t frame = 0; frame < numFrames; ++frame) {
        for (size_t ch = 0; ch < pImpl->numChannels; ++ch) {
            size_t index = frame * pImpl->numChannels + ch;
            buffer[index] = pImpl->blockers[ch].process(buffer[index]);
        }
    }
}

void MultiChannelDCBlocker::processInterleaved(const float* input, float* output, size_t numFrames) {
    for (size_t frame = 0; frame < numFrames; ++frame) {
        for (size_t ch = 0; ch < pImpl->numChannels; ++ch) {
            size_t index = frame * pImpl->numChannels + ch;
            output[index] = pImpl->blockers[ch].process(input[index]);
        }
    }
}

void MultiChannelDCBlocker::processNonInterleaved(float** buffers, size_t numFrames) {
    for (size_t ch = 0; ch < pImpl->numChannels; ++ch) {
        pImpl->blockers[ch].process(buffers[ch], numFrames);
    }
}

} // namespace nap
