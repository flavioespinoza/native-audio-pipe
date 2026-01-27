#ifndef NAP_MIXERNODE_H
#define NAP_MIXERNODE_H

#include "../../api/IAudioNode.h"
#include <memory>

namespace nap {

/**
 * @brief Audio node that mixes multiple input signals into a single output.
 */
class MixerNode : public IAudioNode {
public:
    explicit MixerNode(std::uint32_t numInputs = 2);
    ~MixerNode() override;

    MixerNode(const MixerNode&) = delete;
    MixerNode& operator=(const MixerNode&) = delete;
    MixerNode(MixerNode&&) noexcept;
    MixerNode& operator=(MixerNode&&) noexcept;

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

    // MixerNode specific
    void setInputGain(std::uint32_t inputIndex, float gain);
    float getInputGain(std::uint32_t inputIndex) const;
    void setMasterGain(float gain);
    float getMasterGain() const;
    std::uint32_t getNumInputs() const;
    void muteInput(std::uint32_t inputIndex, bool mute);
    bool isInputMuted(std::uint32_t inputIndex) const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_MIXERNODE_H
