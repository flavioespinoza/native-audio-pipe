#ifndef NAP_DCBLOCKERNODE_H
#define NAP_DCBLOCKERNODE_H

#include "../../api/IAudioNode.h"
#include <memory>

namespace nap {

/**
 * @brief Audio utility node that removes DC offset from the signal.
 */
class DCBlockerNode : public IAudioNode {
public:
    DCBlockerNode();
    ~DCBlockerNode() override;

    DCBlockerNode(const DCBlockerNode&) = delete;
    DCBlockerNode& operator=(const DCBlockerNode&) = delete;
    DCBlockerNode(DCBlockerNode&&) noexcept;
    DCBlockerNode& operator=(DCBlockerNode&&) noexcept;

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

    void setCutoffFrequency(float frequencyHz);
    float getCutoffFrequency() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_DCBLOCKERNODE_H
