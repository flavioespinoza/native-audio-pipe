#ifndef NAP_BENCH_LIMITER_H
#define NAP_BENCH_LIMITER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>

namespace nap {
namespace bench {

struct BenchmarkResult;

/**
 * @brief Limiter benchmark for zero-allocation performance verification
 */
class Bench_Limiter {
public:
    Bench_Limiter();
    ~Bench_Limiter();

    void setBufferSize(size_t size);
    void setSampleRate(double sampleRate);
    void setIterations(uint64_t iterations);

    BenchmarkResult runLimitingBenchmark();
    BenchmarkResult runLookaheadBenchmark();
    BenchmarkResult runTruePeakBenchmark();

    bool verifyZeroAllocation();
    std::string generateReport() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace bench
} // namespace nap

#endif // NAP_BENCH_LIMITER_H
