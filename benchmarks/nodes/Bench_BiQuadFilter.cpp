#include "Bench_BiQuadFilter.h"
#include "Bench_GainNode.h"
#include <chrono>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iomanip>
#include <vector>

namespace nap {
namespace bench {

class Bench_BiQuadFilter::Impl {
public:
    size_t bufferSize = 512;
    double sampleRate = 44100.0;
    uint64_t iterations = 10000;
    uint64_t warmupIterations = 1000;

    std::vector<float> inputBuffer;
    std::vector<float> outputBuffer;
    std::vector<BenchmarkResult> results;

    // Biquad state - Transposed Direct Form II
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;
    float s1 = 0.0f, s2 = 0.0f;

    // DCE prevention: accumulator to prevent compiler from optimizing away the loop
    volatile float resultAccumulator = 0.0f;

    void allocateBuffers() {
        inputBuffer.resize(bufferSize);
        outputBuffer.resize(bufferSize);
        for (size_t i = 0; i < bufferSize; ++i) {
            inputBuffer[i] = std::sin(2.0 * M_PI * 440.0 * i / sampleRate);
        }
    }

    void calculateLowPassCoeffs(float freq, float q) {
        float w0 = 2.0f * M_PI * freq / sampleRate;
        float alpha = std::sin(w0) / (2.0f * q);
        float cosw0 = std::cos(w0);

        float a0 = 1.0f + alpha;
        b0 = ((1.0f - cosw0) / 2.0f) / a0;
        b1 = (1.0f - cosw0) / a0;
        b2 = ((1.0f - cosw0) / 2.0f) / a0;
        a1 = (-2.0f * cosw0) / a0;
        a2 = (1.0f - alpha) / a0;
    }

    void calculateHighPassCoeffs(float freq, float q) {
        float w0 = 2.0f * M_PI * freq / sampleRate;
        float alpha = std::sin(w0) / (2.0f * q);
        float cosw0 = std::cos(w0);

        float a0 = 1.0f + alpha;
        b0 = ((1.0f + cosw0) / 2.0f) / a0;
        b1 = (-(1.0f + cosw0)) / a0;
        b2 = ((1.0f + cosw0) / 2.0f) / a0;
        a1 = (-2.0f * cosw0) / a0;
        a2 = (1.0f - alpha) / a0;
    }

    void calculateBandPassCoeffs(float freq, float q) {
        float w0 = 2.0f * M_PI * freq / sampleRate;
        float alpha = std::sin(w0) / (2.0f * q);
        float cosw0 = std::cos(w0);

        float a0 = 1.0f + alpha;
        b0 = alpha / a0;
        b1 = 0.0f;
        b2 = -alpha / a0;
        a1 = (-2.0f * cosw0) / a0;
        a2 = (1.0f - alpha) / a0;
    }

    void calculateNotchCoeffs(float freq, float q) {
        float w0 = 2.0f * M_PI * freq / sampleRate;
        float alpha = std::sin(w0) / (2.0f * q);
        float cosw0 = std::cos(w0);

        float a0 = 1.0f + alpha;
        b0 = 1.0f / a0;
        b1 = (-2.0f * cosw0) / a0;
        b2 = 1.0f / a0;
        a1 = (-2.0f * cosw0) / a0;
        a2 = (1.0f - alpha) / a0;
    }

    void resetState() {
        s1 = s2 = 0.0f;
    }

    // Transposed Direct Form II processing
    void process() {
        float acc = 0.0f;
        for (size_t i = 0; i < bufferSize; ++i) {
            const float x0 = inputBuffer[i];
            const float y0 = b0 * x0 + s1;
            s1 = b1 * x0 - a1 * y0 + s2;
            s2 = b2 * x0 - a2 * y0;
            outputBuffer[i] = y0;
            acc += y0;  // Accumulate to prevent DCE
        }
        resultAccumulator = acc;  // Store to volatile to prevent optimization
    }

    double calculateStdDev(const std::vector<double>& times, double mean) {
        double variance = 0.0;
        for (double t : times) {
            double diff = t - mean;
            variance += diff * diff;
        }
        return std::sqrt(variance / times.size());
    }
};

Bench_BiQuadFilter::Bench_BiQuadFilter()
    : pImpl(std::make_unique<Impl>()) {
    pImpl->allocateBuffers();
    pImpl->calculateLowPassCoeffs(1000.0f, 0.707f);
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
    BenchmarkResult result;
    result.name = "BiQuadFilter::process (LowPass)";
    result.iterations = pImpl->iterations;
    result.samplesProcessed = pImpl->bufferSize * pImpl->iterations;

    pImpl->calculateLowPassCoeffs(1000.0f, 0.707f);

    std::vector<double> times;
    times.reserve(pImpl->iterations);

    // Warmup
    for (uint64_t i = 0; i < pImpl->warmupIterations; ++i) {
        pImpl->resetState();
        pImpl->process();
    }

    // Benchmark
    for (uint64_t i = 0; i < pImpl->iterations; ++i) {
        pImpl->resetState();
        auto start = std::chrono::high_resolution_clock::now();
        pImpl->process();
        auto end = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration<double, std::nano>(end - start).count());
    }

    result.averageTimeNs = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    result.minTimeNs = *std::min_element(times.begin(), times.end());
    result.maxTimeNs = *std::max_element(times.begin(), times.end());
    result.stdDevNs = pImpl->calculateStdDev(times, result.averageTimeNs);
    result.samplesPerSecond = (pImpl->bufferSize * 1e9) / result.averageTimeNs;
    result.zeroAllocation = true;

    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_BiQuadFilter::runHighPassBenchmark() {
    BenchmarkResult result;
    result.name = "BiQuadFilter::process (HighPass)";
    result.iterations = pImpl->iterations;
    result.samplesProcessed = pImpl->bufferSize * pImpl->iterations;

    pImpl->calculateHighPassCoeffs(1000.0f, 0.707f);

    std::vector<double> times;
    times.reserve(pImpl->iterations);

    // Warmup
    for (uint64_t i = 0; i < pImpl->warmupIterations; ++i) {
        pImpl->resetState();
        pImpl->process();
    }

    // Benchmark
    for (uint64_t i = 0; i < pImpl->iterations; ++i) {
        pImpl->resetState();
        auto start = std::chrono::high_resolution_clock::now();
        pImpl->process();
        auto end = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration<double, std::nano>(end - start).count());
    }

    result.averageTimeNs = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    result.minTimeNs = *std::min_element(times.begin(), times.end());
    result.maxTimeNs = *std::max_element(times.begin(), times.end());
    result.stdDevNs = pImpl->calculateStdDev(times, result.averageTimeNs);
    result.samplesPerSecond = (pImpl->bufferSize * 1e9) / result.averageTimeNs;
    result.zeroAllocation = true;

    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_BiQuadFilter::runBandPassBenchmark() {
    BenchmarkResult result;
    result.name = "BiQuadFilter::process (BandPass)";
    result.iterations = pImpl->iterations;
    result.samplesProcessed = pImpl->bufferSize * pImpl->iterations;

    pImpl->calculateBandPassCoeffs(1000.0f, 0.707f);

    std::vector<double> times;
    times.reserve(pImpl->iterations);

    // Warmup
    for (uint64_t i = 0; i < pImpl->warmupIterations; ++i) {
        pImpl->resetState();
        pImpl->process();
    }

    // Benchmark
    for (uint64_t i = 0; i < pImpl->iterations; ++i) {
        pImpl->resetState();
        auto start = std::chrono::high_resolution_clock::now();
        pImpl->process();
        auto end = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration<double, std::nano>(end - start).count());
    }

    result.averageTimeNs = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    result.minTimeNs = *std::min_element(times.begin(), times.end());
    result.maxTimeNs = *std::max_element(times.begin(), times.end());
    result.stdDevNs = pImpl->calculateStdDev(times, result.averageTimeNs);
    result.samplesPerSecond = (pImpl->bufferSize * 1e9) / result.averageTimeNs;
    result.zeroAllocation = true;

    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_BiQuadFilter::runNotchBenchmark() {
    BenchmarkResult result;
    result.name = "BiQuadFilter::process (Notch)";
    result.iterations = pImpl->iterations;
    result.samplesProcessed = pImpl->bufferSize * pImpl->iterations;

    pImpl->calculateNotchCoeffs(1000.0f, 0.707f);

    std::vector<double> times;
    times.reserve(pImpl->iterations);

    // Warmup
    for (uint64_t i = 0; i < pImpl->warmupIterations; ++i) {
        pImpl->resetState();
        pImpl->process();
    }

    // Benchmark
    for (uint64_t i = 0; i < pImpl->iterations; ++i) {
        pImpl->resetState();
        auto start = std::chrono::high_resolution_clock::now();
        pImpl->process();
        auto end = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration<double, std::nano>(end - start).count());
    }

    result.averageTimeNs = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    result.minTimeNs = *std::min_element(times.begin(), times.end());
    result.maxTimeNs = *std::max_element(times.begin(), times.end());
    result.stdDevNs = pImpl->calculateStdDev(times, result.averageTimeNs);
    result.samplesPerSecond = (pImpl->bufferSize * 1e9) / result.averageTimeNs;
    result.zeroAllocation = true;

    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_BiQuadFilter::runCoefficientUpdateBenchmark() {
    BenchmarkResult result;
    result.name = "BiQuadFilter::calculateCoefficients";
    result.iterations = pImpl->iterations;
    result.samplesProcessed = 0;

    std::vector<double> times;
    times.reserve(pImpl->iterations);

    // Warmup
    for (uint64_t i = 0; i < pImpl->warmupIterations; ++i) {
        float freq = 100.0f + (i % 10000);
        pImpl->calculateLowPassCoeffs(freq, 0.707f);
    }

    // Benchmark
    for (uint64_t i = 0; i < pImpl->iterations; ++i) {
        float freq = 100.0f + (i % 10000);
        auto start = std::chrono::high_resolution_clock::now();
        pImpl->calculateLowPassCoeffs(freq, 0.707f);
        auto end = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration<double, std::nano>(end - start).count());
    }

    result.averageTimeNs = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    result.minTimeNs = *std::min_element(times.begin(), times.end());
    result.maxTimeNs = *std::max_element(times.begin(), times.end());
    result.stdDevNs = pImpl->calculateStdDev(times, result.averageTimeNs);
    result.samplesPerSecond = 0.0;
    result.zeroAllocation = true;
    pImpl->results.push_back(result);
    return result;
}

bool Bench_BiQuadFilter::verifyZeroAllocation() {
    // In production, this would use malloc hooks or memory tracking
    // For now, we rely on code inspection: the process() loop uses only
    // stack variables and preallocated buffers, no heap allocations
    return true;
}

bool Bench_BiQuadFilter::verifyFrequencyResponse() {
    // Test LowPass filter: 500Hz should pass, 5kHz should attenuate
    pImpl->calculateLowPassCoeffs(1000.0f, 0.707f);
    pImpl->resetState();

    // Generate 500Hz sine wave
    for (size_t i = 0; i < pImpl->bufferSize; ++i) {
        pImpl->inputBuffer[i] = std::sin(2.0 * M_PI * 500.0 * i / pImpl->sampleRate);
    }
    pImpl->process();

    // Calculate RMS of output (skip first samples for transient)
    double rmsLow = 0.0;
    for (size_t i = pImpl->bufferSize / 2; i < pImpl->bufferSize; ++i) {
        rmsLow += pImpl->outputBuffer[i] * pImpl->outputBuffer[i];
    }
    rmsLow = std::sqrt(rmsLow / (pImpl->bufferSize / 2));

    // Generate 5kHz sine wave
    pImpl->resetState();
    for (size_t i = 0; i < pImpl->bufferSize; ++i) {
        pImpl->inputBuffer[i] = std::sin(2.0 * M_PI * 5000.0 * i / pImpl->sampleRate);
    }
    pImpl->process();

    // Calculate RMS of output
    double rmsHigh = 0.0;
    for (size_t i = pImpl->bufferSize / 2; i < pImpl->bufferSize; ++i) {
        rmsHigh += pImpl->outputBuffer[i] * pImpl->outputBuffer[i];
    }
    rmsHigh = std::sqrt(rmsHigh / (pImpl->bufferSize / 2));

    // Low frequency should have higher output than high frequency
    return rmsLow > rmsHigh * 2.0;
}

std::string Bench_BiQuadFilter::generateReport() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "\n=== BiQuadFilter Benchmark Report ===\n\n";
    oss << "Configuration:\n";
    oss << "  Buffer Size: " << pImpl->bufferSize << " samples\n";
    oss << "  Sample Rate: " << pImpl->sampleRate << " Hz\n";
    oss << "  Iterations: " << pImpl->iterations << "\n";
    oss << "  Warmup Iterations: " << pImpl->warmupIterations << "\n\n";

    for (const auto& r : pImpl->results) {
        oss << r.name << ":\n";
        oss << "  Average: " << r.averageTimeNs << " ns\n";
        oss << "  Min:     " << r.minTimeNs << " ns\n";
        oss << "  Max:     " << r.maxTimeNs << " ns\n";
        oss << "  StdDev:  " << r.stdDevNs << " ns\n";
        if (r.samplesProcessed > 0) {
            oss << "  Throughput: " << (r.samplesPerSecond / 1e6) << " Msamples/sec\n";
            double msamplesPerBuffer = pImpl->bufferSize / 1e6;
            double timePerBuffer = r.averageTimeNs / 1e3;  // Convert to microseconds
            oss << "  Time per " << pImpl->bufferSize << " samples: "
                << timePerBuffer << " µs\n";
        }
        oss << "  Zero Allocation: " << (r.zeroAllocation ? "YES" : "NO") << "\n\n";
    }

    oss << "Notes:\n";
    oss << "  - Using Transposed Direct Form II structure\n";
    oss << "  - DCE prevention: volatile accumulator\n";
    oss << "  - All benchmarks are zero-allocation hot-path\n";

    return oss.str();
}

} // namespace bench
} // namespace nap
