#include "utils/dsp/Resampler.h"
#include <cmath>
#include <algorithm>

namespace nap {

class Resampler::Impl {
public:
    ResamplerQuality quality = ResamplerQuality::Medium;
    double ratio = 1.0;
    double phase = 0.0;
    std::vector<float> history;
    int filterTaps = 4;

    Impl() {
        updateFilterTaps();
        history.resize(filterTaps * 2, 0.0f);
    }

    void updateFilterTaps() {
        switch (quality) {
            case ResamplerQuality::Fast:   filterTaps = 2; break;
            case ResamplerQuality::Medium: filterTaps = 4; break;
            case ResamplerQuality::High:   filterTaps = 8; break;
            case ResamplerQuality::Best:   filterTaps = 32; break;
        }
        history.resize(filterTaps * 2, 0.0f);
    }

    // Windowed sinc function
    float sinc(float x) const {
        if (std::abs(x) < 1e-6f) return 1.0f;
        float pi_x = static_cast<float>(M_PI) * x;
        return std::sin(pi_x) / pi_x;
    }

    // Kaiser window
    float kaiser(float n, float N, float beta) const {
        float alpha = (N - 1) / 2.0f;
        float arg = beta * std::sqrt(1.0f - std::pow((n - alpha) / alpha, 2.0f));
        return besselI0(arg) / besselI0(beta);
    }

    float besselI0(float x) const {
        float sum = 1.0f;
        float term = 1.0f;
        float x2 = x * x * 0.25f;
        for (int k = 1; k < 20; ++k) {
            term *= x2 / (k * k);
            sum += term;
        }
        return sum;
    }

    // Linear interpolation
    float interpolateLinear(const float* samples, float frac) const {
        return samples[0] + frac * (samples[1] - samples[0]);
    }

    // Cubic interpolation
    float interpolateCubic(const float* samples, float frac) const {
        float a0 = samples[1];
        float a1 = 0.5f * (samples[2] - samples[0]);
        float a2 = samples[0] - 2.5f * samples[1] + 2.0f * samples[2] - 0.5f * samples[3];
        float a3 = 0.5f * (samples[3] - samples[0]) + 1.5f * (samples[1] - samples[2]);

        return ((a3 * frac + a2) * frac + a1) * frac + a0;
    }

    // Windowed sinc interpolation
    float interpolateSinc(const float* samples, float frac, int taps) const {
        float sum = 0.0f;
        float normSum = 0.0f;
        float beta = 8.6f;  // Kaiser beta

        int halfTaps = taps / 2;
        for (int i = -halfTaps + 1; i <= halfTaps; ++i) {
            float x = static_cast<float>(i) - frac;
            float w = kaiser(static_cast<float>(i + halfTaps), static_cast<float>(taps), beta);
            float s = sinc(x) * w;
            sum += samples[i + halfTaps - 1] * s;
            normSum += s;
        }

        return normSum > 0.0f ? sum / normSum : 0.0f;
    }
};

Resampler::Resampler()
    : pImpl(std::make_unique<Impl>()) {}

Resampler::Resampler(ResamplerQuality quality)
    : pImpl(std::make_unique<Impl>()) {
    pImpl->quality = quality;
    pImpl->updateFilterTaps();
}

Resampler::~Resampler() = default;

Resampler::Resampler(Resampler&&) noexcept = default;
Resampler& Resampler::operator=(Resampler&&) noexcept = default;

void Resampler::setQuality(ResamplerQuality quality) {
    pImpl->quality = quality;
    pImpl->updateFilterTaps();
}

ResamplerQuality Resampler::getQuality() const {
    return pImpl->quality;
}

std::vector<float> Resampler::resample(const std::vector<float>& input,
                                       double inputSampleRate,
                                       double outputSampleRate) {
    return resample(input.data(), input.size(), inputSampleRate, outputSampleRate);
}

std::vector<float> Resampler::resample(const float* input, size_t inputSize,
                                       double inputSampleRate,
                                       double outputSampleRate) {
    if (inputSize == 0) return {};

    double ratio = outputSampleRate / inputSampleRate;
    size_t outputSize = static_cast<size_t>(std::ceil(inputSize * ratio));
    std::vector<float> output(outputSize);

    double srcPhase = 0.0;
    double phaseIncrement = 1.0 / ratio;

    int halfTaps = pImpl->filterTaps / 2;

    // Pad input for filter access
    std::vector<float> paddedInput(inputSize + pImpl->filterTaps, 0.0f);
    std::copy(input, input + inputSize, paddedInput.begin() + halfTaps);

    for (size_t i = 0; i < outputSize; ++i) {
        size_t srcIndex = static_cast<size_t>(srcPhase);
        float frac = static_cast<float>(srcPhase - srcIndex);

        if (srcIndex + halfTaps >= paddedInput.size()) break;

        const float* samples = &paddedInput[srcIndex];

        switch (pImpl->quality) {
            case ResamplerQuality::Fast:
                output[i] = pImpl->interpolateLinear(samples + halfTaps - 1, frac);
                break;
            case ResamplerQuality::Medium:
                output[i] = pImpl->interpolateCubic(samples + halfTaps - 2, frac);
                break;
            case ResamplerQuality::High:
            case ResamplerQuality::Best:
                output[i] = pImpl->interpolateSinc(samples, frac, pImpl->filterTaps);
                break;
        }

        srcPhase += phaseIncrement;
    }

    return output;
}

void Resampler::reset() {
    pImpl->phase = 0.0;
    std::fill(pImpl->history.begin(), pImpl->history.end(), 0.0f);
}

void Resampler::setRatio(double inputSampleRate, double outputSampleRate) {
    pImpl->ratio = inputSampleRate / outputSampleRate;
}

double Resampler::getRatio() const {
    return pImpl->ratio;
}

size_t Resampler::process(const float* input, size_t inputFrames,
                          float* output, size_t outputFrames) {
    size_t outputIndex = 0;
    size_t inputIndex = 0;

    while (outputIndex < outputFrames && inputIndex < inputFrames) {
        size_t srcIndex = static_cast<size_t>(pImpl->phase);
        if (srcIndex >= inputFrames) break;

        float frac = static_cast<float>(pImpl->phase - srcIndex);

        // Simple linear interpolation for streaming
        if (srcIndex + 1 < inputFrames) {
            output[outputIndex] = input[srcIndex] + frac * (input[srcIndex + 1] - input[srcIndex]);
        } else {
            output[outputIndex] = input[srcIndex];
        }

        outputIndex++;
        pImpl->phase += pImpl->ratio;
    }

    // Adjust phase for consumed input
    pImpl->phase -= inputFrames;
    if (pImpl->phase < 0) pImpl->phase = 0;

    return outputIndex;
}

size_t Resampler::getOutputSize(size_t inputSize, double ratio) {
    return static_cast<size_t>(std::ceil(inputSize * ratio));
}

size_t Resampler::getInputSize(size_t outputSize, double ratio) {
    return static_cast<size_t>(std::ceil(outputSize / ratio));
}

size_t Resampler::getLatency() const {
    return pImpl->filterTaps / 2;
}

std::vector<float> Resampler::upsample(const std::vector<float>& input, int factor) {
    if (factor <= 1) return input;

    std::vector<float> output(input.size() * factor, 0.0f);
    for (size_t i = 0; i < input.size(); ++i) {
        output[i * factor] = input[i];
    }

    // TODO: Apply anti-imaging filter
    return output;
}

std::vector<float> Resampler::downsample(const std::vector<float>& input, int factor) {
    if (factor <= 1) return input;

    // TODO: Apply anti-aliasing filter first

    std::vector<float> output(input.size() / factor);
    for (size_t i = 0; i < output.size(); ++i) {
        output[i] = input[i * factor];
    }

    return output;
}

} // namespace nap
