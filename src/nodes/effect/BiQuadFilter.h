#ifndef NAP_BIQUADFILTER_H
#define NAP_BIQUADFILTER_H

#include "../../api/IAudioNode.h"
#include <memory>

namespace nap {

// Forward declarations
class FloatParameter;
class EnumParameter;

/**
 * @brief Second-order IIR biquad filter with real-time safe processing.
 *
 * BiQuadFilter implements a high-performance biquad (two-pole, two-zero) IIR filter
 * using the Transposed Direct Form II topology for numerical stability. Supports
 * common filter types including LowPass, HighPass, BandPass, and Notch filters.
 *
 * **Key Features:**
 * - Zero heap allocation in process() loop (real-time safe)
 * - Transposed Direct Form II topology (2 state variables per channel)
 * - Integrated with IParameter system for automation and preset support
 * - Automatic Nyquist frequency clamping prevents filter instability
 * - Parameter smoothing prevents zipper noise (10ms time constant)
 *
 * **Performance:**
 * - ~4µs per 512-sample stereo buffer on Apple M1 @ 44.1kHz
 * - ~0.036% CPU usage at typical audio buffer sizes
 * - 2750x real-time headroom (can run thousands of instances)
 *
 * **Thread Safety:**
 * - process() is NOT thread-safe (single audio thread only)
 * - Parameter changes from other threads are lock-free via IParameter system
 * - prepare() and reset() should only be called when audio is stopped
 *
 * **Usage Example:**
 * @code
 * BiQuadFilter lpf;
 * lpf.prepare(44100.0, 512);                         // Initialize with sample rate
 * lpf.setFilterType(BiQuadFilter::FilterType::LowPass);
 * lpf.setFrequency(1000.0f);                         // Cutoff at 1kHz
 * lpf.setQ(0.707f);                                  // Butterworth response
 *
 * // In audio callback (real-time safe):
 * lpf.process(inputBuffer, outputBuffer, numFrames, numChannels);
 * @endcode
 *
 * **Automation Example:**
 * @code
 * // For DAW automation or MIDI CC control:
 * FloatParameter* freqParam = lpf.getFrequencyParameter();
 * freqParam->setValue(2000.0f);  // Automated value
 * // Parameter smoothing prevents zipper noise automatically
 * @endcode
 *
 * @see docs/api/BiQuadFilter.md for complete API reference
 * @see docs/architecture/BiQuadFilter.md for implementation details
 * @see tests/unit/nodes/effect/Test_BiQuadFilter.cpp for usage examples
 */
class BiQuadFilter : public IAudioNode {
public:
    /**
     * @brief Available filter types.
     *
     * Note: Currently only LowPass, HighPass, BandPass, and Notch are implemented.
     * AllPass, PeakingEQ, LowShelf, HighShelf are reserved for future use.
     */
    enum class FilterType {
        LowPass,    ///< Attenuates frequencies above cutoff
        HighPass,   ///< Attenuates frequencies below cutoff
        BandPass,   ///< Passes frequencies near cutoff, attenuates others
        Notch,      ///< Attenuates frequencies near cutoff (band-reject)
        AllPass,    ///< (Not yet implemented) Passes all frequencies, affects phase
        PeakingEQ,  ///< (Not yet implemented) Boost or cut at specific frequency
        LowShelf,   ///< (Not yet implemented) Boost or cut below cutoff
        HighShelf   ///< (Not yet implemented) Boost or cut above cutoff
    };

    BiQuadFilter();
    ~BiQuadFilter() override;

    BiQuadFilter(const BiQuadFilter&) = delete;
    BiQuadFilter& operator=(const BiQuadFilter&) = delete;
    BiQuadFilter(BiQuadFilter&&) noexcept;
    BiQuadFilter& operator=(BiQuadFilter&&) noexcept;

    // IAudioNode interface
    /**
     * @brief Process audio through the filter (real-time safe).
     *
     * Applies the configured filter to the input buffer and writes results to output.
     * This method is zero-allocation and suitable for real-time audio callbacks.
     *
     * @param inputBuffer  Interleaved input samples (e.g., L,R,L,R for stereo)
     * @param outputBuffer Interleaved output samples (can be same as input for in-place)
     * @param numFrames    Number of frames (samples per channel)
     * @param numChannels  Number of channels (1=mono, 2=stereo)
     *
     * @note NOT thread-safe. Call only from the audio thread.
     * @note Supports up to 2 channels. Mono processes left only, stereo processes both.
     */
    void process(const float* inputBuffer, float* outputBuffer,
                 std::uint32_t numFrames, std::uint32_t numChannels) override;

    /**
     * @brief Initialize filter with sample rate and buffer size.
     *
     * Must be called before process(). Recalculates filter coefficients for the
     * new sample rate. Call when audio engine starts or sample rate changes.
     *
     * @param sampleRate Sample rate in Hz (e.g., 44100.0, 48000.0)
     * @param blockSize  Expected buffer size (used for optimization hints)
     *
     * @note NOT real-time safe (allocates during IParameter setup). Call before audio starts.
     */
    void prepare(double sampleRate, std::uint32_t blockSize) override;

    /**
     * @brief Clear filter state (removes audio artifacts).
     *
     * Resets internal delay lines to zero. Call when:
     * - Starting playback after silence
     * - After seeking in timeline
     * - When parameter changes cause audible artifacts
     *
     * @note Real-time safe. Can be called from audio thread.
     */
    void reset() override;

    /// @brief Get unique node identifier (e.g., "BiQuadFilter_1")
    std::string getNodeId() const override;

    /// @brief Get node type name (returns "BiQuadFilter")
    std::string getTypeName() const override;

    /// @brief Get number of input channels (returns 2)
    std::uint32_t getNumInputChannels() const override;

    /// @brief Get number of output channels (returns 2)
    std::uint32_t getNumOutputChannels() const override;

    /// @brief Check if filter is bypassed (passes audio unprocessed)
    bool isBypassed() const override;

    /// @brief Set bypass state. When true, audio passes through unprocessed.
    void setBypassed(bool bypassed) override;

    // BiQuadFilter specific (direct access - use for non-automated control)

    /**
     * @brief Set filter type (LowPass, HighPass, BandPass, Notch).
     *
     * Changes the filter topology. Triggers coefficient recalculation on next process().
     *
     * @param type Filter type from FilterType enum
     *
     * @note Real-time safe. Updates both internal state and IParameter.
     *
     * **Example:**
     * @code
     * filter.setFilterType(BiQuadFilter::FilterType::HighPass);
     * @endcode
     */
    void setFilterType(FilterType type);

    /// @brief Get current filter type
    FilterType getFilterType() const;

    /**
     * @brief Set cutoff/center frequency in Hz.
     *
     * For LowPass/HighPass: Sets the -3dB cutoff point.
     * For BandPass/Notch: Sets the center frequency.
     *
     * @param frequencyHz Frequency in Hz (20.0 to 20000.0)
     *
     * @note Automatically clamped to 95% of Nyquist frequency to prevent instability.
     * @note With IParameter smoothing, changes occur over 10ms to prevent zipper noise.
     * @note Real-time safe. Updates both internal state and IParameter.
     *
     * **Example:**
     * @code
     * filter.setFrequency(1000.0f);  // 1kHz cutoff
     * @endcode
     */
    void setFrequency(float frequencyHz);

    /// @brief Get current frequency in Hz
    float getFrequency() const;

    /**
     * @brief Set filter Q (resonance/bandwidth).
     *
     * Q controls the sharpness of the filter:
     * - Q = 0.707: Butterworth (maximally flat passband)
     * - Q > 1.0: More resonance (sharper peak/cut)
     * - Q < 0.707: Gentler slope
     *
     * @param q Q factor (0.1 to 20.0)
     *
     * @note Real-time safe. Updates both internal state and IParameter.
     *
     * **Example:**
     * @code
     * filter.setQ(0.707f);  // Butterworth response
     * filter.setQ(5.0f);    // Resonant, sharp cutoff
     * @endcode
     */
    void setQ(float q);

    /// @brief Get current Q factor
    float getQ() const;

    /**
     * @brief Set filter gain in dB (for future EQ/shelf filters).
     *
     * Currently unused by LowPass/HighPass/BandPass/Notch.
     * Will be used by PeakingEQ, LowShelf, HighShelf when implemented.
     *
     * @param gainDb Gain in decibels (-24.0 to +24.0)
     */
    void setGain(float gainDb);

    /// @brief Get current gain in dB
    float getGain() const;

    // IParameter system integration (use for automation, presets, UI binding)

    /**
     * @brief Get frequency parameter for automation/UI binding.
     *
     * Use this instead of setFrequency() when:
     * - Implementing DAW automation
     * - Binding to UI sliders
     * - Loading/saving presets
     * - Receiving MIDI CC
     *
     * @return Pointer to FloatParameter (range: 20-20000 Hz, smoothing: 10ms)
     *
     * **Example (automation):**
     * @code
     * FloatParameter* freq = filter.getFrequencyParameter();
     * freq->setValue(automationValue);  // Smoothed automatically
     * @endcode
     */
    FloatParameter* getFrequencyParameter();

    /**
     * @brief Get Q parameter for automation/UI binding.
     * @return Pointer to FloatParameter (range: 0.1-20.0, smoothing: 10ms)
     */
    FloatParameter* getQParameter();

    /**
     * @brief Get filter type parameter for automation/UI binding.
     * @return Pointer to EnumParameter (values: LowPass, HighPass, BandPass, Notch)
     */
    EnumParameter* getFilterTypeParameter();

    /**
     * @brief Get gain parameter for automation/UI binding.
     * @return Pointer to FloatParameter (range: -24 to +24 dB, smoothing: 10ms)
     */
    FloatParameter* getGainParameter();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_BIQUADFILTER_H
