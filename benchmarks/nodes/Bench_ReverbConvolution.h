#ifndef NAP_BENCH_REVERB_CONVOLUTION_H
#define NAP_BENCH_REVERB_CONVOLUTION_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>

namespace nap {
namespace bench {

struct BenchmarkResult;

/**
 * @brief ReverbConvolution benchmark for zero-allocation performance verification
 */
class Bench_ReverbConvolution {
public:
    Bench_ReverbConvolution();
    ~Bench_ReverbConvolution();

    // Configuration
    void setBufferSize(size_t size);
    void setImpulseResponseSize(size_t size);
    void setSampleRate(double sampleRate);
    void setIterations(uint64_t iterations);

    // Run benchmarks
    BenchmarkResult runConvolutionBenchmark();
    BenchmarkResult runFFTConvolutionBenchmark();
    BenchmarkResult runPartitionedConvolutionBenchmark();
    BenchmarkResult runIRLoadBenchmark();

    // Verification
    bool verifyZeroAllocation();
    bool verifyConvolutionAccuracy();

    // Report
    std::string generateReport() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace bench
} // namespace nap

#endif // NAP_BENCH_REVERB_CONVOLUTION_H
