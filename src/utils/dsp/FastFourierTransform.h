#ifndef NAP_FAST_FOURIER_TRANSFORM_H
#define NAP_FAST_FOURIER_TRANSFORM_H

#include <memory>
#include <vector>
#include <complex>
#include <cstddef>

namespace nap {

/**
 * @brief Fast Fourier Transform wrapper class
 *
 * Provides efficient FFT/IFFT operations for spectral analysis
 * and frequency-domain processing. Supports power-of-two sizes.
 */
class FastFourierTransform {
public:
    using Complex = std::complex<float>;

    explicit FastFourierTransform(size_t size);
    ~FastFourierTransform();

    // Non-copyable, movable
    FastFourierTransform(const FastFourierTransform&) = delete;
    FastFourierTransform& operator=(const FastFourierTransform&) = delete;
    FastFourierTransform(FastFourierTransform&&) noexcept;
    FastFourierTransform& operator=(FastFourierTransform&&) noexcept;

    // Configuration
    size_t getSize() const;
    void setSize(size_t size);

    // Forward FFT (time -> frequency)
    void forward(const float* input, Complex* output);
    void forward(const float* input, float* magnitudes, float* phases);
    std::vector<Complex> forward(const std::vector<float>& input);

    // Inverse FFT (frequency -> time)
    void inverse(const Complex* input, float* output);
    void inverse(const float* magnitudes, const float* phases, float* output);
    std::vector<float> inverse(const std::vector<Complex>& input);

    // Real-to-complex optimized
    void forwardReal(const float* input, Complex* output);
    void inverseReal(const Complex* input, float* output);

    // Utility
    static size_t getFrequencyBin(float frequency, float sampleRate, size_t fftSize);
    static float getBinFrequency(size_t bin, float sampleRate, size_t fftSize);
    static bool isPowerOfTwo(size_t n);
    static size_t nextPowerOfTwo(size_t n);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nap

#endif // NAP_FAST_FOURIER_TRANSFORM_H
