#ifndef NAP_SOFTCLIPPER_H
#define NAP_SOFTCLIPPER_H

#include "../../api/IAudioNode.h"
#include <memory>

namespace nap {

/**
 * @brief Audio effect node that applies soft clipping (saturation) distortion.
 */
class SoftClipper : public IAudioNode {
public:
    enum class CurveType { Tanh, Atan, Cubic, Sine };

    SoftClipper();
    ~SoftClipper() override;

    SoftClipper(const SoftClipper&) = delete;
    SoftClipper& operator=(const SoftClipper&) = delete;
    SoftClipper(SoftClipper&&) noexcept;
    SoftClipper& operator=(SoftClipper&&) noexcept;

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

    void setDrive(float drive);
    float getDrive() const;
    void setCurveType(CurveType type);
    CurveType getCurveType() const;
    void setMix(float mix);
    float getMix() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_SOFTCLIPPER_H
