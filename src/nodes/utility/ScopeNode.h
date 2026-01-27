#ifndef NAP_SCOPENODE_H
#define NAP_SCOPENODE_H

#include "../../api/IAudioNode.h"
#include <memory>
#include <vector>

namespace nap {

/**
 * @brief Audio utility node for capturing waveform data for visualization.
 */
class ScopeNode : public IAudioNode {
public:
    ScopeNode();
    ~ScopeNode() override;

    ScopeNode(const ScopeNode&) = delete;
    ScopeNode& operator=(const ScopeNode&) = delete;
    ScopeNode(ScopeNode&&) noexcept;
    ScopeNode& operator=(ScopeNode&&) noexcept;

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

    void setBufferSize(std::size_t samples);
    std::size_t getBufferSize() const;
    std::vector<float> getWaveformData(std::uint32_t channel) const;
    void setTriggerLevel(float level);
    float getTriggerLevel() const;
    void setTriggerEnabled(bool enabled);
    bool isTriggerEnabled() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_SCOPENODE_H
