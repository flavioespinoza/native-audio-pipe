#ifndef NAP_BENCH_COMPRESSOR_H
#define NAP_BENCH_COMPRESSOR_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>

namespace nap {
namespace bench {

struct BenchmarkResult;

/**
 * @brief Compressor benchmark for zero-allocation performance verification
 */
class Bench_Compressor {
public:
    Bench_Compressor();
    ~Bench_Compressor();

    void setBufferSize(size_t size);
    void setSampleRate(double sampleRate);
    void setIterations(uint64_t iterations);

    BenchmarkResult runCompressionBenchmark();
    BenchmarkResult runEnvelopeFollowerBenchmark();
    BenchmarkResult runGainComputationBenchmark();
    BenchmarkResult runSidechainBenchmark();

    bool verifyZeroAllocation();
    std::string generateReport() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace bench
} // namespace nap

#endif // NAP_BENCH_COMPRESSOR_H
