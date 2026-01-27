#ifndef NAP_RINGMODULATOR_H
#define NAP_RINGMODULATOR_H

#include "../../api/IAudioNode.h"
#include <memory>

namespace nap {

/**
 * @brief Audio effect node that applies ring modulation.
 */
class RingModulator : public IAudioNode {
public:
    enum class WaveformType { Sine, Square, Triangle, Sawtooth };

    RingModulator();
    ~RingModulator() override;

    RingModulator(const RingModulator&) = delete;
    RingModulator& operator=(const RingModulator&) = delete;
    RingModulator(RingModulator&&) noexcept;
    RingModulator& operator=(RingModulator&&) noexcept;

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

    void setFrequency(float frequencyHz);
    float getFrequency() const;
    void setWaveform(WaveformType type);
    WaveformType getWaveform() const;
    void setMix(float mix);
    float getMix() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_RINGMODULATOR_H
