#ifndef NAP_BENCH_GAIN_NODE_H
#define NAP_BENCH_GAIN_NODE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace nap {
namespace bench {

/**
 * @brief Benchmark results structure
 */
struct BenchmarkResult {
    std::string name;
    double averageTimeNs;
    double minTimeNs;
    double maxTimeNs;
    double stdDevNs;
    uint64_t iterations;
    size_t samplesProcessed;
    double samplesPerSecond;
    bool zeroAllocation;
};

/**
 * @brief GainNode benchmark for zero-allocation performance verification
 */
class Bench_GainNode {
public:
    Bench_GainNode();
    ~Bench_GainNode();

    // Configuration
    void setBufferSize(size_t size);
    void setSampleRate(double sampleRate);
    void setIterations(uint64_t iterations);
    void setWarmupIterations(uint64_t warmup);

    // Run benchmarks
    BenchmarkResult runProcessBenchmark();
    BenchmarkResult runGainChangeBenchmark();
    BenchmarkResult runSIMDBenchmark();

    // Verification
    bool verifyZeroAllocation();
    bool verifyOutputCorrectness();

    // Report
    std::string generateReport() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace bench
} // namespace nap

#endif // NAP_BENCH_GAIN_NODE_H
