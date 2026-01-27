#ifndef NAP_SINEOSCILLATOR_H
#define NAP_SINEOSCILLATOR_H

#include "../../api/IAudioNode.h"
#include "../../api/IAudioSource.h"
#include <memory>

namespace nap {

/**
 * @brief Audio source node that generates a sine wave.
 */
class SineOscillator : public IAudioNode, public IAudioSource {
public:
    SineOscillator();
    ~SineOscillator() override;

    SineOscillator(const SineOscillator&) = delete;
    SineOscillator& operator=(const SineOscillator&) = delete;
    SineOscillator(SineOscillator&&) noexcept;
    SineOscillator& operator=(SineOscillator&&) noexcept;

    // IAudioNode interface
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

    // IAudioSource interface
    void generate(float* outputBuffer, std::uint32_t numFrames, std::uint32_t numChannels) override;
    bool hasMoreData() const override;
    std::uint64_t getTotalSamples() const override;
    std::uint64_t getCurrentPosition() const override;
    bool seek(std::uint64_t positionSamples) override;
    bool isSeekable() const override;
    void start() override;
    void stop() override;
    bool isActive() const override;

    // SineOscillator specific
    void setFrequency(float frequencyHz);
    float getFrequency() const;
    void setAmplitude(float amplitude);
    float getAmplitude() const;
    void setPhase(float phaseRadians);
    float getPhase() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_SINEOSCILLATOR_H
