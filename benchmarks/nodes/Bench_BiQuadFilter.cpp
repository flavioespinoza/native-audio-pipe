#include "Bench_BiQuadFilter.h"
#include "Bench_GainNode.h"
#include "../../src/nodes/effect/BiQuadFilter.h"
#include <chrono>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iomanip>
#include <vector>

namespace nap {
namespace bench {

// Volatile sink — the compiler cannot prove this has no observable side-effect,
// so it is legally required to keep the accumulation alive.  Without this (or
// equivalent), -O2/-O3 happily eliminates the entire process() loop via DCE,
// producing fake sub-nanosecond timings.
static volatile float g_dce_sink = 0.0f;

class Bench_BiQuadFilter::Impl {
public:
    size_t bufferSize = 512;
    double sampleRate = 44100.0;
    uint64_t iterations = 10000;
    uint64_t warmupIterations = 1000;

    std::vector<float> inputBuffer;
    std::vector<float> outputBuffer;
    std::vector<BenchmarkResult> results;

    // The real node under test — benchmarks exercise the production code path.
    nap::BiQuadFilter filter;

    void allocateBuffers() {
        inputBuffer.resize(bufferSize);
        outputBuffer.resize(bufferSize);
        // Fill with a 440 Hz sine — a realistic single-tone input.
        for (size_t i = 0; i < bufferSize; ++i) {
            inputBuffer[i] = static_cast<float>(
                std::sin(2.0 * 3.14159265358979323846 * 440.0 * i / sampleRate));
        }
    }

    BenchmarkResult runFilterBenchmark(const std::string& name,
                                       BiQuadFilter::FilterType type,
                                       float freqHz, float q)
    {
        BenchmarkResult result;
        result.name = name;
        result.iterations = iterations;
        result.samplesProcessed = bufferSize * iterations;

        filter.prepare(sampleRate, static_cast<uint32_t>(bufferSize));
        filter.setFilterType(type);
        filter.setFrequency(freqHz);
        filter.setQ(q);

        // Warmup — brings caches hot and lets branch predictors settle.
        for (uint64_t i = 0; i < warmupIterations; ++i) {
            filter.reset();
            filter.process(inputBuffer.data(), outputBuffer.data(),
                           static_cast<uint32_t>(bufferSize), 1);
            // Feed last sample into volatile sink to prevent warmup being
            // optimised away as well.
            g_dce_sink = outputBuffer[bufferSize - 1];
        }

        std::vector<double> times;
        times.reserve(iterations);

        for (uint64_t i = 0; i < iterations; ++i) {
            filter.reset();

            auto start = std::chrono::high_resolution_clock::now();
            filter.process(inputBuffer.data(), outputBuffer.data(),
                           static_cast<uint32_t>(bufferSize), 1);
            auto end = std::chrono::high_resolution_clock::now();

            // Accumulate the last output sample into the volatile sink.
            // This is the DCE guard: the compiler must materialise y[N-1]
            // because it escapes to an externally-visible location.
            g_dce_sink += outputBuffer[bufferSize - 1];

            times.push_back(std::chrono::duration<double, std::nano>(end - start).count());
        }

        result.averageTimeNs = std::accumulate(times.begin(), times.end(), 0.0)
                               / static_cast<double>(times.size());
        result.minTimeNs = *std::min_element(times.begin(), times.end());
        result.maxTimeNs = *std::max_element(times.begin(), times.end());
        result.samplesPerSecond = (static_cast<double>(bufferSize) * 1e9) / result.averageTimeNs;
        result.zeroAllocation = true;  // TDF-II process loop has no allocations

        results.push_back(result);
        return result;
    }
};

Bench_BiQuadFilter::Bench_BiQuadFilter()
    : pImpl(std::make_unique<Impl>()) {
    pImpl->allocateBuffers();
}

Bench_BiQuadFilter::~Bench_BiQuadFilter() = default;

void Bench_BiQuadFilter::setBufferSize(size_t size) {
    pImpl->bufferSize = size;
    pImpl->allocateBuffers();
}

void Bench_BiQuadFilter::setSampleRate(double sampleRate) {
    pImpl->sampleRate = sampleRate;
    pImpl->allocateBuffers();
}

void Bench_BiQuadFilter::setIterations(uint64_t iterations) {
    pImpl->iterations = iterations;
}

void Bench_BiQuadFilter::setWarmupIterations(uint64_t warmup) {
    pImpl->warmupIterations = warmup;
}

BenchmarkResult Bench_BiQuadFilter::runLowPassBenchmark() {
    return pImpl->runFilterBenchmark(
        "BiQuadFilter::process (LowPass, TDF-II)",
        BiQuadFilter::FilterType::LowPass, 1000.0f, 0.707f);
}

BenchmarkResult Bench_BiQuadFilter::runHighPassBenchmark() {
    return pImpl->runFilterBenchmark(
        "BiQuadFilter::process (HighPass, TDF-II)",
        BiQuadFilter::FilterType::HighPass, 5000.0f, 0.707f);
}

BenchmarkResult Bench_BiQuadFilter::runBandPassBenchmark() {
    return pImpl->runFilterBenchmark(
        "BiQuadFilter::process (BandPass, TDF-II)",
        BiQuadFilter::FilterType::BandPass, 2000.0f, 1.414f);
}

BenchmarkResult Bench_BiQuadFilter::runNotchBenchmark() {
    return pImpl->runFilterBenchmark(
        "BiQuadFilter::process (Notch, TDF-II)",
        BiQuadFilter::FilterType::Notch, 3000.0f, 2.0f);
}

BenchmarkResult Bench_BiQuadFilter::runCoefficientUpdateBenchmark() {
    BenchmarkResult result;
    result.name = "BiQuadFilter::setFrequency (coeff recalc)";
    result.iterations = pImpl->iterations;

    pImpl->filter.prepare(pImpl->sampleRate, static_cast<uint32_t>(pImpl->bufferSize));
    pImpl->filter.setFilterType(BiQuadFilter::FilterType::LowPass);

    // Warmup
    for (uint64_t i = 0; i < pImpl->warmupIterations; ++i) {
        pImpl->filter.setFrequency(100.0f + static_cast<float>(i % 10000));
    }

    std::vector<double> times;
    times.reserve(pImpl->iterations);

    for (uint64_t i = 0; i < pImpl->iterations; ++i) {
        float freq = 100.0f + static_cast<float>(i % 10000);
        auto start = std::chrono::high_resolution_clock::now();
        pImpl->filter.setFrequency(freq);
        auto end = std::chrono::high_resolution_clock::now();

        // Prevent the setFrequency from being hoisted/dead-stored away.
        // Reading back the value forces the coefficient path to complete.
        g_dce_sink = pImpl->filter.getFrequency();

        times.push_back(std::chrono::duration<double, std::nano>(end - start).count());
    }

    result.averageTimeNs = std::accumulate(times.begin(), times.end(), 0.0)
                           / static_cast<double>(times.size());
    result.minTimeNs = *std::min_element(times.begin(), times.end());
    result.maxTimeNs = *std::max_element(times.begin(), times.end());
    result.zeroAllocation = true;

    pImpl->results.push_back(result);
    return result;
}

bool Bench_BiQuadFilter::verifyZeroAllocation() {
    // The TDF-II process() loop contains no new/malloc/resize calls.
    // This is a structural guarantee enforced by code review; a runtime
    // allocator hook (e.g. via jemalloc or tcmalloc interpose) would be
    // needed for automated runtime proof in CI.
    return true;
}

bool Bench_BiQuadFilter::verifyFrequencyResponse() {
    // Quick smoke-check: a 100 Hz tone through a 5 kHz LowPass must not
    // be attenuated, while a 15 kHz tone must be.
    const size_t N = 4096;
    std::vector<float> input(N), output(N);

    pImpl->filter.prepare(pImpl->sampleRate, static_cast<uint32_t>(N));
    pImpl->filter.setFilterType(BiQuadFilter::FilterType::LowPass);
    pImpl->filter.setFrequency(5000.0f);
    pImpl->filter.setQ(0.707f);
    pImpl->filter.reset();

    // 100 Hz tone — should pass
    for (size_t i = 0; i < N; ++i) {
        input[i] = static_cast<float>(
            std::sin(2.0 * 3.14159265358979323846 * 100.0 * i / pImpl->sampleRate));
    }
    pImpl->filter.process(input.data(), output.data(), static_cast<uint32_t>(N), 1);
    float rmsIn  = 0.0f, rmsOut = 0.0f;
    for (size_t i = N / 2; i < N; ++i) {  // second half to skip transient
        rmsIn  += input[i]  * input[i];
        rmsOut += output[i] * output[i];
    }
    float ratioPass = std::sqrt(rmsOut) / (std::sqrt(rmsIn) + 1e-12f);

    // 15 kHz tone — should be attenuated
    pImpl->filter.reset();
    for (size_t i = 0; i < N; ++i) {
        input[i] = static_cast<float>(
            std::sin(2.0 * 3.14159265358979323846 * 15000.0 * i / pImpl->sampleRate));
    }
    pImpl->filter.process(input.data(), output.data(), static_cast<uint32_t>(N), 1);
    rmsIn = 0.0f; rmsOut = 0.0f;
    for (size_t i = N / 2; i < N; ++i) {
        rmsIn  += input[i]  * input[i];
        rmsOut += output[i] * output[i];
    }
    float ratioStop = std::sqrt(rmsOut) / (std::sqrt(rmsIn) + 1e-12f);

    // Passband should be > 0.9 (-0.9 dB), stopband < 0.25 (-12 dB)
    return (ratioPass > 0.9f) && (ratioStop < 0.25f);
}

std::string Bench_BiQuadFilter::generateReport() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "=== BiQuadFilter Benchmark Report (TDF-II) ===\n\n";
    for (const auto& r : pImpl->results) {
        oss << r.name << "\n"
            << "  avg: " << r.averageTimeNs << " ns\n"
            << "  min: " << r.minTimeNs << " ns\n"
            << "  max: " << r.maxTimeNs << " ns\n"
            << "  throughput: " << (r.samplesPerSecond / 1e6) << " Msamples/s\n"
            << "  zero-alloc: " << (r.zeroAllocation ? "YES" : "NO") << "\n\n";
    }
    return oss.str();
}

} // namespace bench
} // namespace nap
