#include "utils/dsp/WindowFunctions.h"
#include <cmath>
#include <algorithm>

namespace nap {

class WindowFunctions::Impl {
public:
    // Cache for frequently used windows
};

WindowFunctions::WindowFunctions()
    : pImpl(std::make_unique<Impl>()) {}

WindowFunctions::~WindowFunctions() = default;

WindowFunctions::WindowFunctions(WindowFunctions&&) noexcept = default;
WindowFunctions& WindowFunctions::operator=(WindowFunctions&&) noexcept = default;

std::vector<float> WindowFunctions::generate(WindowType type, size_t size, float param) {
    switch (type) {
        case WindowType::Rectangular:   return rectangular(size);
        case WindowType::Hann:          return hann(size);
        case WindowType::Hamming:       return hamming(size);
        case WindowType::Blackman:      return blackman(size);
        case WindowType::BlackmanHarris: return blackmanHarris(size);
        case WindowType::Kaiser:        return kaiser(size, param > 0 ? param : 8.6f);
        case WindowType::Triangular:    return triangular(size);
        case WindowType::Gaussian:      return gaussian(size, param > 0 ? param : 0.4f);
        case WindowType::FlatTop:       return flatTop(size);
        case WindowType::Tukey:         return tukey(size, param > 0 ? param : 0.5f);
        default:                        return rectangular(size);
    }
}

std::vector<float> WindowFunctions::rectangular(size_t size) {
    return std::vector<float>(size, 1.0f);
}

std::vector<float> WindowFunctions::hann(size_t size) {
    std::vector<float> window(size);
    for (size_t i = 0; i < size; ++i) {
        window[i] = 0.5f * (1.0f - std::cos(2.0f * static_cast<float>(M_PI) * i / (size - 1)));
    }
    return window;
}

std::vector<float> WindowFunctions::hamming(size_t size) {
    std::vector<float> window(size);
    for (size_t i = 0; i < size; ++i) {
        window[i] = 0.54f - 0.46f * std::cos(2.0f * static_cast<float>(M_PI) * i / (size - 1));
    }
    return window;
}

std::vector<float> WindowFunctions::blackman(size_t size) {
    std::vector<float> window(size);
    const float a0 = 0.42f;
    const float a1 = 0.5f;
    const float a2 = 0.08f;

    for (size_t i = 0; i < size; ++i) {
        float n = static_cast<float>(i) / (size - 1);
        window[i] = a0 - a1 * std::cos(2.0f * static_cast<float>(M_PI) * n) +
                    a2 * std::cos(4.0f * static_cast<float>(M_PI) * n);
    }
    return window;
}

std::vector<float> WindowFunctions::blackmanHarris(size_t size) {
    std::vector<float> window(size);
    const float a0 = 0.35875f;
    const float a1 = 0.48829f;
    const float a2 = 0.14128f;
    const float a3 = 0.01168f;

    for (size_t i = 0; i < size; ++i) {
        float n = static_cast<float>(i) / (size - 1);
        window[i] = a0 - a1 * std::cos(2.0f * static_cast<float>(M_PI) * n) +
                    a2 * std::cos(4.0f * static_cast<float>(M_PI) * n) -
                    a3 * std::cos(6.0f * static_cast<float>(M_PI) * n);
    }
    return window;
}

float WindowFunctions::besselI0(float x) {
    // Modified Bessel function of the first kind, order 0
    float sum = 1.0f;
    float term = 1.0f;
    float x2 = x * x * 0.25f;

    for (int k = 1; k < 25; ++k) {
        term *= x2 / (k * k);
        sum += term;
        if (term < 1e-10f * sum) break;
    }

    return sum;
}

std::vector<float> WindowFunctions::kaiser(size_t size, float beta) {
    std::vector<float> window(size);
    float denominator = besselI0(beta);

    for (size_t i = 0; i < size; ++i) {
        float n = 2.0f * i / (size - 1) - 1.0f;
        float arg = beta * std::sqrt(1.0f - n * n);
        window[i] = besselI0(arg) / denominator;
    }
    return window;
}

std::vector<float> WindowFunctions::triangular(size_t size) {
    std::vector<float> window(size);
    float L = static_cast<float>(size);

    for (size_t i = 0; i < size; ++i) {
        window[i] = 1.0f - std::abs((i - (L - 1) / 2.0f) / (L / 2.0f));
    }
    return window;
}

std::vector<float> WindowFunctions::gaussian(size_t size, float sigma) {
    std::vector<float> window(size);
    float N = static_cast<float>(size - 1);

    for (size_t i = 0; i < size; ++i) {
        float n = (i - N / 2.0f) / (sigma * N / 2.0f);
        window[i] = std::exp(-0.5f * n * n);
    }
    return window;
}

std::vector<float> WindowFunctions::flatTop(size_t size) {
    std::vector<float> window(size);
    const float a0 = 0.21557895f;
    const float a1 = 0.41663158f;
    const float a2 = 0.277263158f;
    const float a3 = 0.083578947f;
    const float a4 = 0.006947368f;

    for (size_t i = 0; i < size; ++i) {
        float n = static_cast<float>(i) / (size - 1);
        window[i] = a0 - a1 * std::cos(2.0f * static_cast<float>(M_PI) * n) +
                    a2 * std::cos(4.0f * static_cast<float>(M_PI) * n) -
                    a3 * std::cos(6.0f * static_cast<float>(M_PI) * n) +
                    a4 * std::cos(8.0f * static_cast<float>(M_PI) * n);
    }
    return window;
}

std::vector<float> WindowFunctions::tukey(size_t size, float alpha) {
    std::vector<float> window(size);
    size_t taperLength = static_cast<size_t>(alpha * (size - 1) / 2.0f);

    for (size_t i = 0; i < size; ++i) {
        if (i < taperLength) {
            window[i] = 0.5f * (1.0f + std::cos(static_cast<float>(M_PI) *
                (2.0f * i / (alpha * (size - 1)) - 1.0f)));
        } else if (i >= size - taperLength) {
            window[i] = 0.5f * (1.0f + std::cos(static_cast<float>(M_PI) *
                (2.0f * i / (alpha * (size - 1)) - 2.0f / alpha + 1.0f)));
        } else {
            window[i] = 1.0f;
        }
    }
    return window;
}

void WindowFunctions::apply(float* signal, const float* window, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        signal[i] *= window[i];
    }
}

void WindowFunctions::apply(float* signal, WindowType type, size_t size, float param) {
    auto window = generate(type, size, param);
    apply(signal, window.data(), size);
}

std::vector<float> WindowFunctions::apply(const std::vector<float>& signal, WindowType type, float param) {
    auto window = generate(type, signal.size(), param);
    std::vector<float> result = signal;
    apply(result.data(), window.data(), signal.size());
    return result;
}

float WindowFunctions::getCoherentGain(WindowType type) {
    switch (type) {
        case WindowType::Rectangular:    return 1.0f;
        case WindowType::Hann:           return 0.5f;
        case WindowType::Hamming:        return 0.54f;
        case WindowType::Blackman:       return 0.42f;
        case WindowType::BlackmanHarris: return 0.36f;
        default:                         return 1.0f;
    }
}

float WindowFunctions::getEquivalentNoiseBandwidth(WindowType type) {
    switch (type) {
        case WindowType::Rectangular:    return 1.0f;
        case WindowType::Hann:           return 1.5f;
        case WindowType::Hamming:        return 1.36f;
        case WindowType::Blackman:       return 1.73f;
        case WindowType::BlackmanHarris: return 2.0f;
        default:                         return 1.0f;
    }
}

float WindowFunctions::getScallopingLoss(WindowType type) {
    switch (type) {
        case WindowType::Rectangular:    return 3.92f;
        case WindowType::Hann:           return 1.42f;
        case WindowType::Hamming:        return 1.78f;
        case WindowType::Blackman:       return 1.1f;
        case WindowType::BlackmanHarris: return 0.83f;
        default:                         return 3.92f;
    }
}

float WindowFunctions::getHighestSidelobeLevel(WindowType type) {
    switch (type) {
        case WindowType::Rectangular:    return -13.0f;
        case WindowType::Hann:           return -31.5f;
        case WindowType::Hamming:        return -42.7f;
        case WindowType::Blackman:       return -58.1f;
        case WindowType::BlackmanHarris: return -92.0f;
        default:                         return -13.0f;
    }
}

float WindowFunctions::getSidelobeFalloff(WindowType type) {
    switch (type) {
        case WindowType::Rectangular:    return -6.0f;
        case WindowType::Hann:           return -18.0f;
        case WindowType::Hamming:        return -6.0f;
        case WindowType::Blackman:       return -18.0f;
        case WindowType::BlackmanHarris: return -6.0f;
        default:                         return -6.0f;
    }
}

float WindowFunctions::getRecommendedOverlap(WindowType type) {
    switch (type) {
        case WindowType::Rectangular:    return 0.0f;
        case WindowType::Hann:           return 0.5f;
        case WindowType::Hamming:        return 0.5f;
        case WindowType::Blackman:       return 0.67f;
        case WindowType::BlackmanHarris: return 0.75f;
        case WindowType::Triangular:     return 0.5f;
        default:                         return 0.5f;
    }
}

std::vector<float> WindowFunctions::generateCOLA(WindowType type, size_t size, float overlap) {
    // Generate Constant Overlap-Add compliant window
    auto window = generate(type, size);

    // Normalize for COLA compliance
    size_t hopSize = static_cast<size_t>(size * (1.0f - overlap));
    float sum = 0.0f;

    // Calculate sum at hop positions
    for (size_t i = 0; i < size; i += hopSize) {
        sum += window[i];
    }

    // Normalize
    if (sum > 0.0f) {
        float scale = 1.0f / sum;
        for (float& w : window) {
            w *= scale;
        }
    }

    return window;
}

} // namespace nap
