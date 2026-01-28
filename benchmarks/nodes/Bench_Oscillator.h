#ifndef NAP_BENCH_OSCILLATOR_H
#define NAP_BENCH_OSCILLATOR_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>

namespace nap {
namespace bench {

struct BenchmarkResult;

/**
 * @brief Oscillator benchmark for zero-allocation performance verification
 */
class Bench_Oscillator {
public:
    Bench_Oscillator();
    ~Bench_Oscillator();

    // Configuration
    void setBufferSize(size_t size);
    void setSampleRate(double sampleRate);
    void setIterations(uint64_t iterations);

    // Run benchmarks
    BenchmarkResult runSineBenchmark();
    BenchmarkResult runSawBenchmark();
    BenchmarkResult runSquareBenchmark();
    BenchmarkResult runTriangleBenchmark();
    BenchmarkResult runWavetableBenchmark();
    BenchmarkResult runFMBenchmark();

    // Verification
    bool verifyZeroAllocation();
    bool verifyWaveformAccuracy();

    // Report
    std::string generateReport() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace bench
} // namespace nap

#endif // NAP_BENCH_OSCILLATOR_H
