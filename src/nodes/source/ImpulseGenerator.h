#ifndef NAP_IMPULSEGENERATOR_H
#define NAP_IMPULSEGENERATOR_H

#include "../../api/IAudioNode.h"
#include "../../api/IAudioSource.h"
#include <memory>

namespace nap {

/**
 * @brief Audio source node that generates impulse signals.
 */
class ImpulseGenerator : public IAudioNode, public IAudioSource {
public:
    ImpulseGenerator();
    ~ImpulseGenerator() override;

    ImpulseGenerator(const ImpulseGenerator&) = delete;
    ImpulseGenerator& operator=(const ImpulseGenerator&) = delete;
    ImpulseGenerator(ImpulseGenerator&&) noexcept;
    ImpulseGenerator& operator=(ImpulseGenerator&&) noexcept;

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

    // ImpulseGenerator specific
    void setAmplitude(float amplitude);
    float getAmplitude() const;
    void setIntervalSamples(std::uint32_t samples);
    std::uint32_t getIntervalSamples() const;
    void setIntervalMs(float milliseconds);
    float getIntervalMs() const;
    void trigger();
    void setOneShot(bool oneShot);
    bool isOneShot() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_IMPULSEGENERATOR_H
