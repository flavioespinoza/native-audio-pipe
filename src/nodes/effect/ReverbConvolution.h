#ifndef NAP_REVERBCONVOLUTION_H
#define NAP_REVERBCONVOLUTION_H

#include "../../api/IAudioNode.h"
#include <memory>
#include <string>

namespace nap {

/**
 * @brief Audio effect node that applies convolution reverb using impulse responses.
 */
class ReverbConvolution : public IAudioNode {
public:
    ReverbConvolution();
    ~ReverbConvolution() override;

    ReverbConvolution(const ReverbConvolution&) = delete;
    ReverbConvolution& operator=(const ReverbConvolution&) = delete;
    ReverbConvolution(ReverbConvolution&&) noexcept;
    ReverbConvolution& operator=(ReverbConvolution&&) noexcept;

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

    bool loadImpulseResponse(const std::string& filePath);
    bool loadImpulseResponse(const float* irData, std::size_t irLength);
    void unloadImpulseResponse();
    bool isImpulseResponseLoaded() const;
    std::size_t getImpulseResponseLength() const;
    void setDryWetMix(float mix);
    float getDryWetMix() const;
    void setPreDelay(float milliseconds);
    float getPreDelay() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_REVERBCONVOLUTION_H
