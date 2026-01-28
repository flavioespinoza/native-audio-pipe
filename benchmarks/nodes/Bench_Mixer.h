#ifndef NAP_BENCH_MIXER_H
#define NAP_BENCH_MIXER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>

namespace nap {
namespace bench {

struct BenchmarkResult;

/**
 * @brief Mixer benchmark for zero-allocation performance verification
 */
class Bench_Mixer {
public:
    Bench_Mixer();
    ~Bench_Mixer();

    void setBufferSize(size_t size);
    void setChannelCount(size_t channels);
    void setSampleRate(double sampleRate);
    void setIterations(uint64_t iterations);

    BenchmarkResult runStereoMixBenchmark();
    BenchmarkResult runMultiChannelMixBenchmark();
    BenchmarkResult runPanningBenchmark();
    BenchmarkResult runGainStagingBenchmark();

    bool verifyZeroAllocation();
    std::string generateReport() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace bench
} // namespace nap

#endif // NAP_BENCH_MIXER_H
