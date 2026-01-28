#ifndef NAP_BENCH_GATE_H
#define NAP_BENCH_GATE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>

namespace nap {
namespace bench {

struct BenchmarkResult;

/**
 * @brief Gate benchmark for zero-allocation performance verification
 */
class Bench_Gate {
public:
    Bench_Gate();
    ~Bench_Gate();

    void setBufferSize(size_t size);
    void setSampleRate(double sampleRate);
    void setIterations(uint64_t iterations);

    BenchmarkResult runGatingBenchmark();
    BenchmarkResult runExpanderBenchmark();
    BenchmarkResult runHysteresisBenchmark();

    bool verifyZeroAllocation();
    std::string generateReport() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace bench
} // namespace nap

#endif // NAP_BENCH_GATE_H
