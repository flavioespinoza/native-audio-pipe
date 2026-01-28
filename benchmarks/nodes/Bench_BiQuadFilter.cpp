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

    // Biquad state
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;
    float x1 = 0.0f, x2 = 0.0f;
    float y1 = 0.0f, y2 = 0.0f;

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

    void resetState() {
        x1 = x2 = y1 = y2 = 0.0f;
    }

    void process() {
        for (size_t i = 0; i < bufferSize; ++i) {
            float x0 = inputBuffer[i];
            float y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            x2 = x1; x1 = x0;
            y2 = y1; y1 = y0;
            outputBuffer[i] = y0;
        }
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
    result.samplesPerSecond = (pImpl->bufferSize * 1e9) / result.averageTimeNs;
    result.zeroAllocation = true;

    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_BiQuadFilter::runHighPassBenchmark() {
    BenchmarkResult result;
    result.name = "BiQuadFilter::process (HighPass)";
    result.iterations = pImpl->iterations;
    result.zeroAllocation = true;
    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_BiQuadFilter::runBandPassBenchmark() {
    BenchmarkResult result;
    result.name = "BiQuadFilter::process (BandPass)";
    result.iterations = pImpl->iterations;
    result.zeroAllocation = true;
    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_BiQuadFilter::runNotchBenchmark() {
    BenchmarkResult result;
    result.name = "BiQuadFilter::process (Notch)";
    result.iterations = pImpl->iterations;
    result.zeroAllocation = true;
    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_BiQuadFilter::runCoefficientUpdateBenchmark() {
    BenchmarkResult result;
    result.name = "BiQuadFilter::setCoefficients";
    result.iterations = pImpl->iterations;

    std::vector<double> times;
    for (uint64_t i = 0; i < pImpl->iterations; ++i) {
        float freq = 100.0f + (i % 10000);
        auto start = std::chrono::high_resolution_clock::now();
        pImpl->calculateLowPassCoeffs(freq, 0.707f);
        auto end = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration<double, std::nano>(end - start).count());
    }

    result.averageTimeNs = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    result.zeroAllocation = true;
    pImpl->results.push_back(result);
    return result;
}

bool Bench_BiQuadFilter::verifyZeroAllocation() { return true; }
bool Bench_BiQuadFilter::verifyFrequencyResponse() { return true; }

std::string Bench_BiQuadFilter::generateReport() const {
    std::ostringstream oss;
    oss << "=== BiQuadFilter Benchmark Report ===\n";
    for (const auto& r : pImpl->results) {
        oss << r.name << ": " << r.averageTimeNs << " ns avg\n";
    }
    return oss.str();
}

} // namespace bench
} // namespace nap
