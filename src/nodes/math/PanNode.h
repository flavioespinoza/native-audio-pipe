#ifndef NAP_PANNODE_H
#define NAP_PANNODE_H

#include "../../api/IAudioNode.h"
#include <memory>

namespace nap {

/**
 * @brief Audio node that pans a stereo signal left or right.
 */
class PanNode : public IAudioNode {
public:
    enum class PanLaw {
        Linear,
        ConstantPower,
        MinusFourPointFive
    };

    PanNode();
    ~PanNode() override;

    PanNode(const PanNode&) = delete;
    PanNode& operator=(const PanNode&) = delete;
    PanNode(PanNode&&) noexcept;
    PanNode& operator=(PanNode&&) noexcept;

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

    // PanNode specific
    void setPan(float pan);  // -1.0 (left) to 1.0 (right)
    float getPan() const;
    void setPanLaw(PanLaw law);
    PanLaw getPanLaw() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_PANNODE_H
