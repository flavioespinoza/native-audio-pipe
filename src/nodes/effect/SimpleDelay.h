#ifndef NAP_SIMPLEDELAY_H
#define NAP_SIMPLEDELAY_H

#include "../../api/IAudioNode.h"
#include <memory>

namespace nap {

/**
 * @brief Audio effect node that applies a simple delay to the signal.
 */
class SimpleDelay : public IAudioNode {
public:
    SimpleDelay();
    ~SimpleDelay() override;

    SimpleDelay(const SimpleDelay&) = delete;
    SimpleDelay& operator=(const SimpleDelay&) = delete;
    SimpleDelay(SimpleDelay&&) noexcept;
    SimpleDelay& operator=(SimpleDelay&&) noexcept;

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

    // SimpleDelay specific
    void setDelayTime(float milliseconds);
    float getDelayTime() const;
    void setFeedback(float feedback);
    float getFeedback() const;
    void setMix(float mix);
    float getMix() const;
    void setMaxDelayTime(float milliseconds);
    float getMaxDelayTime() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_SIMPLEDELAY_H
