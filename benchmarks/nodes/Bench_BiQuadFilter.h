#ifndef NAP_BENCH_BIQUAD_FILTER_H
#define NAP_BENCH_BIQUAD_FILTER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>

namespace nap {
namespace bench {

struct BenchmarkResult;

/**
 * @brief BiQuadFilter benchmark for zero-allocation performance verification
 */
class Bench_BiQuadFilter {
public:
    Bench_BiQuadFilter();
    ~Bench_BiQuadFilter();

    // Configuration
    void setBufferSize(size_t size);
    void setSampleRate(double sampleRate);
    void setIterations(uint64_t iterations);
    void setWarmupIterations(uint64_t warmup);

    // Run benchmarks
    BenchmarkResult runLowPassBenchmark();
    BenchmarkResult runHighPassBenchmark();
    BenchmarkResult runBandPassBenchmark();
    BenchmarkResult runNotchBenchmark();
    BenchmarkResult runCoefficientUpdateBenchmark();

    // Verification
    bool verifyZeroAllocation();
    bool verifyFrequencyResponse();

    // Report
    std::string generateReport() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace bench
} // namespace nap

#endif // NAP_BENCH_BIQUAD_FILTER_H
