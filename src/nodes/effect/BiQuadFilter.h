#ifndef NAP_BIQUADFILTER_H
#define NAP_BIQUADFILTER_H

#include "../../api/IAudioNode.h"
#include <memory>

namespace nap {

// Forward declarations
class FloatParameter;
class EnumParameter;

/**
 * @brief Audio effect node implementing a biquad filter.
 */
class BiQuadFilter : public IAudioNode {
public:
    enum class FilterType {
        LowPass,
        HighPass,
        BandPass,
        Notch,
        AllPass,
        PeakingEQ,
        LowShelf,
        HighShelf
    };

    BiQuadFilter();
    ~BiQuadFilter() override;

    BiQuadFilter(const BiQuadFilter&) = delete;
    BiQuadFilter& operator=(const BiQuadFilter&) = delete;
    BiQuadFilter(BiQuadFilter&&) noexcept;
    BiQuadFilter& operator=(BiQuadFilter&&) noexcept;

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

    // BiQuadFilter specific (direct access - use for non-automated control)
    void setFilterType(FilterType type);
    FilterType getFilterType() const;
    void setFrequency(float frequencyHz);
    float getFrequency() const;
    void setQ(float q);
    float getQ() const;
    void setGain(float gainDb);
    float getGain() const;

    // IParameter system integration (use for automation, presets, UI binding)
    FloatParameter* getFrequencyParameter();
    FloatParameter* getQParameter();
    EnumParameter* getFilterTypeParameter();
    FloatParameter* getGainParameter();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_BIQUADFILTER_H
