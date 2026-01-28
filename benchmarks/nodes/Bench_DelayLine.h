#ifndef NAP_BENCH_DELAY_LINE_H
#define NAP_BENCH_DELAY_LINE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>

namespace nap {
namespace bench {

struct BenchmarkResult;

/**
 * @brief DelayLine benchmark for zero-allocation performance verification
 */
class Bench_DelayLine {
public:
    Bench_DelayLine();
    ~Bench_DelayLine();

    // Configuration
    void setBufferSize(size_t size);
    void setMaxDelayTime(float seconds);
    void setSampleRate(double sampleRate);
    void setIterations(uint64_t iterations);

    // Run benchmarks
    BenchmarkResult runFixedDelayBenchmark();
    BenchmarkResult runModulatedDelayBenchmark();
    BenchmarkResult runInterpolatedReadBenchmark();
    BenchmarkResult runFeedbackBenchmark();

    // Verification
    bool verifyZeroAllocation();
    bool verifyDelayAccuracy();

    // Report
    std::string generateReport() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace bench
} // namespace nap

#endif // NAP_BENCH_DELAY_LINE_H
