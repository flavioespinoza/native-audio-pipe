#ifndef NAP_DC_BLOCKER_H
#define NAP_DC_BLOCKER_H

#include <memory>
#include <cstddef>

namespace nap {

/**
 * @brief DC offset removal filter
 *
 * High-pass filter designed specifically to remove DC offset
 * from audio signals with minimal impact on audible frequencies.
 */
class DCBlocker {
public:
    /**
     * @brief Construct DC blocker with default cutoff (10 Hz at 44.1kHz)
     */
    DCBlocker();

    /**
     * @brief Construct DC blocker with specified R coefficient
     * @param R Pole radius (typically 0.995 - 0.9999)
     */
    explicit DCBlocker(float R);

    ~DCBlocker();

    // Non-copyable, movable
    DCBlocker(const DCBlocker&) = delete;
    DCBlocker& operator=(const DCBlocker&) = delete;
    DCBlocker(DCBlocker&&) noexcept;
    DCBlocker& operator=(DCBlocker&&) noexcept;

    // Configuration
    void setR(float R);
    float getR() const;

    void setCutoffFrequency(float frequency, float sampleRate);
    float getCutoffFrequency(float sampleRate) const;

    // Reset state
    void reset();

    // Process single sample
    float process(float input);

    // Process buffer
    void process(float* buffer, size_t numSamples);
    void process(const float* input, float* output, size_t numSamples);

    // Static utility
    static float calculateR(float cutoffFrequency, float sampleRate);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

/**
 * @brief Multi-channel DC blocker
 *
 * Provides independent DC blocking for multiple audio channels.
 */
class MultiChannelDCBlocker {
public:
    explicit MultiChannelDCBlocker(size_t numChannels, float R = 0.995f);
    ~MultiChannelDCBlocker();

    // Non-copyable, movable
    MultiChannelDCBlocker(const MultiChannelDCBlocker&) = delete;
    MultiChannelDCBlocker& operator=(const MultiChannelDCBlocker&) = delete;
    MultiChannelDCBlocker(MultiChannelDCBlocker&&) noexcept;
    MultiChannelDCBlocker& operator=(MultiChannelDCBlocker&&) noexcept;

    // Configuration
    size_t getNumChannels() const;
    void setNumChannels(size_t numChannels);
    void setR(float R);
    void setCutoffFrequency(float frequency, float sampleRate);

    // Reset all channels
    void reset();

    // Process interleaved audio
    void processInterleaved(float* buffer, size_t numFrames);
    void processInterleaved(const float* input, float* output, size_t numFrames);

    // Process non-interleaved (channel pointers)
    void processNonInterleaved(float** buffers, size_t numFrames);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nap

#endif // NAP_DC_BLOCKER_H
