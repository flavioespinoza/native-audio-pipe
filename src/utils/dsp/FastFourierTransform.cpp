#include "utils/dsp/FastFourierTransform.h"
#include <cmath>
#include <algorithm>

namespace nap {

class FastFourierTransform::Impl {
public:
    size_t size;
    std::vector<Complex> twiddleFactors;
    std::vector<Complex> workBuffer;
    std::vector<size_t> bitReversalTable;

    explicit Impl(size_t n) : size(n) {
        computeTwiddleFactors();
        computeBitReversalTable();
        workBuffer.resize(n);
    }

    void computeTwiddleFactors() {
        twiddleFactors.resize(size);
        for (size_t k = 0; k < size; ++k) {
            float angle = -2.0f * static_cast<float>(M_PI) * k / size;
            twiddleFactors[k] = Complex(std::cos(angle), std::sin(angle));
        }
    }

    void computeBitReversalTable() {
        bitReversalTable.resize(size);
        size_t bits = static_cast<size_t>(std::log2(size));
        for (size_t i = 0; i < size; ++i) {
            size_t reversed = 0;
            for (size_t j = 0; j < bits; ++j) {
                if (i & (1 << j)) {
                    reversed |= (1 << (bits - 1 - j));
                }
            }
            bitReversalTable[i] = reversed;
        }
    }

    void bitReverseCopy(const Complex* input, Complex* output) {
        for (size_t i = 0; i < size; ++i) {
            output[bitReversalTable[i]] = input[i];
        }
    }

    void cooleyTukeyFFT(Complex* data, bool inverse) {
        // Bit-reversal permutation (in-place)
        for (size_t i = 0; i < size; ++i) {
            if (i < bitReversalTable[i]) {
                std::swap(data[i], data[bitReversalTable[i]]);
            }
        }

        // Cooley-Tukey iterative FFT
        for (size_t len = 2; len <= size; len *= 2) {
            float angle = (inverse ? 2.0f : -2.0f) * static_cast<float>(M_PI) / len;
            Complex wlen(std::cos(angle), std::sin(angle));

            for (size_t i = 0; i < size; i += len) {
                Complex w(1.0f, 0.0f);
                for (size_t j = 0; j < len / 2; ++j) {
                    Complex u = data[i + j];
                    Complex t = w * data[i + j + len / 2];
                    data[i + j] = u + t;
                    data[i + j + len / 2] = u - t;
                    w *= wlen;
                }
            }
        }

        // Normalize for inverse FFT
        if (inverse) {
            float scale = 1.0f / size;
            for (size_t i = 0; i < size; ++i) {
                data[i] *= scale;
            }
        }
    }
};

FastFourierTransform::FastFourierTransform(size_t size)
    : pImpl(std::make_unique<Impl>(isPowerOfTwo(size) ? size : nextPowerOfTwo(size))) {}

FastFourierTransform::~FastFourierTransform() = default;

FastFourierTransform::FastFourierTransform(FastFourierTransform&&) noexcept = default;
FastFourierTransform& FastFourierTransform::operator=(FastFourierTransform&&) noexcept = default;

size_t FastFourierTransform::getSize() const {
    return pImpl->size;
}

void FastFourierTransform::setSize(size_t size) {
    if (size != pImpl->size) {
        pImpl = std::make_unique<Impl>(isPowerOfTwo(size) ? size : nextPowerOfTwo(size));
    }
}

void FastFourierTransform::forward(const float* input, Complex* output) {
    for (size_t i = 0; i < pImpl->size; ++i) {
        pImpl->workBuffer[i] = Complex(input[i], 0.0f);
    }

    pImpl->cooleyTukeyFFT(pImpl->workBuffer.data(), false);

    std::copy(pImpl->workBuffer.begin(), pImpl->workBuffer.end(), output);
}

void FastFourierTransform::forward(const float* input, float* magnitudes, float* phases) {
    std::vector<Complex> spectrum(pImpl->size);
    forward(input, spectrum.data());

    for (size_t i = 0; i < pImpl->size; ++i) {
        magnitudes[i] = std::abs(spectrum[i]);
        phases[i] = std::arg(spectrum[i]);
    }
}

std::vector<FastFourierTransform::Complex> FastFourierTransform::forward(const std::vector<float>& input) {
    std::vector<Complex> output(pImpl->size);
    std::vector<float> paddedInput(pImpl->size, 0.0f);
    std::copy_n(input.begin(), std::min(input.size(), pImpl->size), paddedInput.begin());
    forward(paddedInput.data(), output.data());
    return output;
}

void FastFourierTransform::inverse(const Complex* input, float* output) {
    std::copy_n(input, pImpl->size, pImpl->workBuffer.begin());

    pImpl->cooleyTukeyFFT(pImpl->workBuffer.data(), true);

    for (size_t i = 0; i < pImpl->size; ++i) {
        output[i] = pImpl->workBuffer[i].real();
    }
}

void FastFourierTransform::inverse(const float* magnitudes, const float* phases, float* output) {
    std::vector<Complex> spectrum(pImpl->size);
    for (size_t i = 0; i < pImpl->size; ++i) {
        spectrum[i] = std::polar(magnitudes[i], phases[i]);
    }
    inverse(spectrum.data(), output);
}

std::vector<float> FastFourierTransform::inverse(const std::vector<Complex>& input) {
    std::vector<float> output(pImpl->size);
    std::vector<Complex> paddedInput(pImpl->size, Complex(0.0f, 0.0f));
    std::copy_n(input.begin(), std::min(input.size(), pImpl->size), paddedInput.begin());
    inverse(paddedInput.data(), output.data());
    return output;
}

void FastFourierTransform::forwardReal(const float* input, Complex* output) {
    // For real input, we can optimize by only computing N/2+1 bins
    forward(input, output);
}

void FastFourierTransform::inverseReal(const Complex* input, float* output) {
    // Exploit conjugate symmetry for real output
    inverse(input, output);
}

size_t FastFourierTransform::getFrequencyBin(float frequency, float sampleRate, size_t fftSize) {
    return static_cast<size_t>(frequency * fftSize / sampleRate + 0.5f);
}

float FastFourierTransform::getBinFrequency(size_t bin, float sampleRate, size_t fftSize) {
    return static_cast<float>(bin) * sampleRate / fftSize;
}

bool FastFourierTransform::isPowerOfTwo(size_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

size_t FastFourierTransform::nextPowerOfTwo(size_t n) {
    if (n == 0) return 1;
    --n;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return n + 1;
}

} // namespace nap
