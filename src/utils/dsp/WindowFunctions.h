#ifndef NAP_WINDOW_FUNCTIONS_H
#define NAP_WINDOW_FUNCTIONS_H

#include <memory>
#include <vector>
#include <cstddef>

namespace nap {

/**
 * @brief Window function types for spectral analysis
 */
enum class WindowType {
    Rectangular,
    Hann,
    Hamming,
    Blackman,
    BlackmanHarris,
    Kaiser,
    Triangular,
    Gaussian,
    FlatTop,
    Tukey
};

/**
 * @brief Window function generator and applicator
 *
 * Provides various window functions for spectral analysis
 * and overlap-add processing.
 */
class WindowFunctions {
public:
    WindowFunctions();
    ~WindowFunctions();

    // Non-copyable, movable
    WindowFunctions(const WindowFunctions&) = delete;
    WindowFunctions& operator=(const WindowFunctions&) = delete;
    WindowFunctions(WindowFunctions&&) noexcept;
    WindowFunctions& operator=(WindowFunctions&&) noexcept;

    // Generate window
    static std::vector<float> generate(WindowType type, size_t size, float param = 0.0f);

    // Individual window generators
    static std::vector<float> rectangular(size_t size);
    static std::vector<float> hann(size_t size);
    static std::vector<float> hamming(size_t size);
    static std::vector<float> blackman(size_t size);
    static std::vector<float> blackmanHarris(size_t size);
    static std::vector<float> kaiser(size_t size, float beta = 8.6f);
    static std::vector<float> triangular(size_t size);
    static std::vector<float> gaussian(size_t size, float sigma = 0.4f);
    static std::vector<float> flatTop(size_t size);
    static std::vector<float> tukey(size_t size, float alpha = 0.5f);

    // Apply window to signal
    static void apply(float* signal, const float* window, size_t size);
    static void apply(float* signal, WindowType type, size_t size, float param = 0.0f);
    static std::vector<float> apply(const std::vector<float>& signal, WindowType type, float param = 0.0f);

    // Window properties
    static float getCoherentGain(WindowType type);
    static float getEquivalentNoiseBandwidth(WindowType type);
    static float getScallopingLoss(WindowType type);
    static float getHighestSidelobeLevel(WindowType type);
    static float getSidelobeFalloff(WindowType type);

    // Overlap-add helpers
    static float getRecommendedOverlap(WindowType type);
    static std::vector<float> generateCOLA(WindowType type, size_t size, float overlap);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;

    // Bessel function for Kaiser window
    static float besselI0(float x);
};

} // namespace nap

#endif // NAP_WINDOW_FUNCTIONS_H
