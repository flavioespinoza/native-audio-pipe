#ifndef NAP_INVERTERNODE_H
#define NAP_INVERTERNODE_H

#include "../../api/IAudioNode.h"
#include <memory>

namespace nap {

/**
 * @brief Audio node that inverts (phase flips) the audio signal.
 */
class InverterNode : public IAudioNode {
public:
    InverterNode();
    ~InverterNode() override;

    InverterNode(const InverterNode&) = delete;
    InverterNode& operator=(const InverterNode&) = delete;
    InverterNode(InverterNode&&) noexcept;
    InverterNode& operator=(InverterNode&&) noexcept;

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

    // InverterNode specific
    void setInvertLeft(bool invert);
    bool getInvertLeft() const;
    void setInvertRight(bool invert);
    bool getInvertRight() const;
    void setInvertBoth(bool invert);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_INVERTERNODE_H
