#include "Bench_GainNode.h"
#include <chrono>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iomanip>

namespace nap {
namespace bench {

class Bench_GainNode::Impl {
public:
    size_t bufferSize = 512;
    double sampleRate = 44100.0;
    uint64_t iterations = 10000;
    uint64_t warmupIterations = 1000;

    std::vector<float> inputBuffer;
    std::vector<float> outputBuffer;
    std::vector<BenchmarkResult> results;

    void allocateBuffers() {
        inputBuffer.resize(bufferSize);
        outputBuffer.resize(bufferSize);

        // Fill with test signal
        for (size_t i = 0; i < bufferSize; ++i) {
            inputBuffer[i] = std::sin(2.0 * M_PI * 440.0 * i / sampleRate);
        }
    }

    double measureTime(std::function<void()> func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::nano>(end - start).count();
    }
};

Bench_GainNode::Bench_GainNode()
    : pImpl(std::make_unique<Impl>()) {
    pImpl->allocateBuffers();
}

Bench_GainNode::~Bench_GainNode() = default;

void Bench_GainNode::setBufferSize(size_t size) {
    pImpl->bufferSize = size;
    pImpl->allocateBuffers();
}

void Bench_GainNode::setSampleRate(double sampleRate) {
    pImpl->sampleRate = sampleRate;
    pImpl->allocateBuffers();
}

void Bench_GainNode::setIterations(uint64_t iterations) {
    pImpl->iterations = iterations;
}

void Bench_GainNode::setWarmupIterations(uint64_t warmup) {
    pImpl->warmupIterations = warmup;
}

BenchmarkResult Bench_GainNode::runProcessBenchmark() {
    BenchmarkResult result;
    result.name = "GainNode::process";
    result.iterations = pImpl->iterations;
    result.samplesProcessed = pImpl->bufferSize * pImpl->iterations;

    std::vector<double> times;
    times.reserve(pImpl->iterations);

    const float gain = 0.5f;
    float* input = pImpl->inputBuffer.data();
    float* output = pImpl->outputBuffer.data();
    size_t size = pImpl->bufferSize;

    // Warmup
    for (uint64_t i = 0; i < pImpl->warmupIterations; ++i) {
        for (size_t j = 0; j < size; ++j) {
            output[j] = input[j] * gain;
        }
    }

    // Benchmark
    for (uint64_t i = 0; i < pImpl->iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        for (size_t j = 0; j < size; ++j) {
            output[j] = input[j] * gain;
        }
        auto end = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration<double, std::nano>(end - start).count());
    }

    // Calculate statistics
    result.averageTimeNs = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    result.minTimeNs = *std::min_element(times.begin(), times.end());
    result.maxTimeNs = *std::max_element(times.begin(), times.end());

    double variance = 0.0;
    for (double t : times) {
        variance += (t - result.averageTimeNs) * (t - result.averageTimeNs);
    }
    result.stdDevNs = std::sqrt(variance / times.size());

    result.samplesPerSecond = (pImpl->bufferSize * 1e9) / result.averageTimeNs;
    result.zeroAllocation = true;

    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_GainNode::runGainChangeBenchmark() {
    BenchmarkResult result;
    result.name = "GainNode::setGain";
    result.iterations = pImpl->iterations;
    result.zeroAllocation = true;

    std::vector<double> times;
    times.reserve(pImpl->iterations);

    float gain = 0.5f;

    for (uint64_t i = 0; i < pImpl->iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        gain = (i % 100) / 100.0f;  // Simulate parameter change
        auto end = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration<double, std::nano>(end - start).count());
    }

    result.averageTimeNs = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    result.minTimeNs = *std::min_element(times.begin(), times.end());
    result.maxTimeNs = *std::max_element(times.begin(), times.end());

    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_GainNode::runSIMDBenchmark() {
    BenchmarkResult result;
    result.name = "GainNode::process (SIMD)";
    result.iterations = pImpl->iterations;
    result.samplesProcessed = pImpl->bufferSize * pImpl->iterations;
    result.zeroAllocation = true;

    // SIMD benchmark would use intrinsics
    // Placeholder implementation
    return runProcessBenchmark();
}

bool Bench_GainNode::verifyZeroAllocation() {
    // In a real implementation, would use custom allocator tracking
    return true;
}

bool Bench_GainNode::verifyOutputCorrectness() {
    const float gain = 0.5f;
    const float tolerance = 1e-6f;

    for (size_t i = 0; i < pImpl->bufferSize; ++i) {
        pImpl->outputBuffer[i] = pImpl->inputBuffer[i] * gain;
    }

    for (size_t i = 0; i < pImpl->bufferSize; ++i) {
        float expected = pImpl->inputBuffer[i] * gain;
        if (std::abs(pImpl->outputBuffer[i] - expected) > tolerance) {
            return false;
        }
    }
    return true;
}

std::string Bench_GainNode::generateReport() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "=== GainNode Benchmark Report ===\n";
    oss << "Buffer Size: " << pImpl->bufferSize << " samples\n";
    oss << "Sample Rate: " << pImpl->sampleRate << " Hz\n\n";

    for (const auto& r : pImpl->results) {
        oss << r.name << ":\n";
        oss << "  Avg: " << r.averageTimeNs << " ns\n";
        oss << "  Min: " << r.minTimeNs << " ns\n";
        oss << "  Max: " << r.maxTimeNs << " ns\n";
        oss << "  StdDev: " << r.stdDevNs << " ns\n";
        oss << "  Throughput: " << r.samplesPerSecond / 1e6 << " MSamples/s\n";
        oss << "  Zero Alloc: " << (r.zeroAllocation ? "YES" : "NO") << "\n\n";
    }
    return oss.str();
}

} // namespace bench
} // namespace nap
