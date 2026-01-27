#ifndef NAP_RESAMPLER_H
#define NAP_RESAMPLER_H

#include <memory>
#include <vector>
#include <cstddef>

namespace nap {

/**
 * @brief Resampling quality modes
 */
enum class ResamplerQuality {
    Fast,       // Linear interpolation
    Medium,     // Cubic interpolation
    High,       // Windowed sinc, 8 taps
    Best        // Windowed sinc, 32 taps
};

/**
 * @brief Sample rate converter
 *
 * Provides high-quality sample rate conversion with
 * multiple quality/performance trade-offs.
 */
class Resampler {
public:
    Resampler();
    explicit Resampler(ResamplerQuality quality);
    ~Resampler();

    // Non-copyable, movable
    Resampler(const Resampler&) = delete;
    Resampler& operator=(const Resampler&) = delete;
    Resampler(Resampler&&) noexcept;
    Resampler& operator=(Resampler&&) noexcept;

    // Configuration
    void setQuality(ResamplerQuality quality);
    ResamplerQuality getQuality() const;

    // Single-shot resampling
    std::vector<float> resample(const std::vector<float>& input,
                                double inputSampleRate,
                                double outputSampleRate);

    std::vector<float> resample(const float* input, size_t inputSize,
                                double inputSampleRate,
                                double outputSampleRate);

    // Streaming resampling
    void reset();
    void setRatio(double inputSampleRate, double outputSampleRate);
    double getRatio() const;

    size_t process(const float* input, size_t inputFrames,
                   float* output, size_t outputFrames);

    // Utility
    static size_t getOutputSize(size_t inputSize, double ratio);
    static size_t getInputSize(size_t outputSize, double ratio);
    size_t getLatency() const;

    // Integer ratio optimization
    static std::vector<float> upsample(const std::vector<float>& input, int factor);
    static std::vector<float> downsample(const std::vector<float>& input, int factor);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nap

#endif // NAP_RESAMPLER_H
