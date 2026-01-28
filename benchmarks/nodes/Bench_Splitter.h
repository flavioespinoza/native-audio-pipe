#ifndef NAP_BENCH_SPLITTER_H
#define NAP_BENCH_SPLITTER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>

namespace nap {
namespace bench {

struct BenchmarkResult;

/**
 * @brief Splitter benchmark for zero-allocation performance verification
 */
class Bench_Splitter {
public:
    Bench_Splitter();
    ~Bench_Splitter();

    void setBufferSize(size_t size);
    void setOutputCount(size_t outputs);
    void setIterations(uint64_t iterations);

    BenchmarkResult runSplitBenchmark();
    BenchmarkResult runCrossfadeBenchmark();

    bool verifyZeroAllocation();
    std::string generateReport() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace bench
} // namespace nap

#endif // NAP_BENCH_SPLITTER_H
