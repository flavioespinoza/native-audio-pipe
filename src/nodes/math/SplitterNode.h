#ifndef NAP_SPLITTERNODE_H
#define NAP_SPLITTERNODE_H

#include "../../api/IAudioNode.h"
#include <memory>

namespace nap {

/**
 * @brief Audio node that splits a single input signal to multiple outputs.
 */
class SplitterNode : public IAudioNode {
public:
    explicit SplitterNode(std::uint32_t numOutputs = 2);
    ~SplitterNode() override;

    SplitterNode(const SplitterNode&) = delete;
    SplitterNode& operator=(const SplitterNode&) = delete;
    SplitterNode(SplitterNode&&) noexcept;
    SplitterNode& operator=(SplitterNode&&) noexcept;

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

    // SplitterNode specific
    std::uint32_t getNumOutputs() const;
    void setOutputGain(std::uint32_t outputIndex, float gain);
    float getOutputGain(std::uint32_t outputIndex) const;
    void muteOutput(std::uint32_t outputIndex, bool mute);
    bool isOutputMuted(std::uint32_t outputIndex) const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_SPLITTERNODE_H
