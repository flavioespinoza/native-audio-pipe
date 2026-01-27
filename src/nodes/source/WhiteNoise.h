#ifndef NAP_WHITENOISE_H
#define NAP_WHITENOISE_H

#include "../../api/IAudioNode.h"
#include "../../api/IAudioSource.h"
#include <memory>

namespace nap {

/**
 * @brief Audio source node that generates white noise.
 */
class WhiteNoise : public IAudioNode, public IAudioSource {
public:
    WhiteNoise();
    ~WhiteNoise() override;

    WhiteNoise(const WhiteNoise&) = delete;
    WhiteNoise& operator=(const WhiteNoise&) = delete;
    WhiteNoise(WhiteNoise&&) noexcept;
    WhiteNoise& operator=(WhiteNoise&&) noexcept;

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

    // WhiteNoise specific
    void setAmplitude(float amplitude);
    float getAmplitude() const;
    void setSeed(std::uint32_t seed);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_WHITENOISE_H
