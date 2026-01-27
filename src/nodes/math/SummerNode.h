#ifndef NAP_SUMMERNODE_H
#define NAP_SUMMERNODE_H

#include "../../api/IAudioNode.h"
#include <memory>

namespace nap {

/**
 * @brief Audio node that sums multiple input signals.
 */
class SummerNode : public IAudioNode {
public:
    explicit SummerNode(std::uint32_t numInputs = 2);
    ~SummerNode() override;

    SummerNode(const SummerNode&) = delete;
    SummerNode& operator=(const SummerNode&) = delete;
    SummerNode(SummerNode&&) noexcept;
    SummerNode& operator=(SummerNode&&) noexcept;

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

    // SummerNode specific
    std::uint32_t getNumInputs() const;
    void setNormalize(bool normalize);
    bool getNormalize() const;
    void setClip(bool clip);
    bool getClip() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_SUMMERNODE_H
