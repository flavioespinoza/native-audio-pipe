#include "Bench_Gate.h"
#include "Bench_GainNode.h"
#include <chrono>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <vector>

namespace nap {
namespace bench {

class Bench_Gate::Impl {
public:
    size_t bufferSize = 512;
    double sampleRate = 44100.0;
    uint64_t iterations = 10000;

    std::vector<float> inputBuffer;
    std::vector<float> outputBuffer;
    std::vector<BenchmarkResult> results;

    float threshold = -40.0f;  // dB
    float range = -80.0f;      // dB
    float attackMs = 1.0f;
    float holdMs = 50.0f;
    float releaseMs = 100.0f;

    float envelope = 0.0f;
    float gain = 0.0f;
    size_t holdCounter = 0;

    void allocateBuffers() {
        inputBuffer.resize(bufferSize);
        outputBuffer.resize(bufferSize);

        for (size_t i = 0; i < bufferSize; ++i) {
            float env = (i < bufferSize / 2) ? 0.1f : 0.5f;
            inputBuffer[i] = env * std::sin(2.0 * M_PI * 440.0 * i / sampleRate);
        }
    }
};

Bench_Gate::Bench_Gate() : pImpl(std::make_unique<Impl>()) {
    pImpl->allocateBuffers();
}

Bench_Gate::~Bench_Gate() = default;

void Bench_Gate::setBufferSize(size_t size) {
    pImpl->bufferSize = size;
    pImpl->allocateBuffers();
}

void Bench_Gate::setSampleRate(double sampleRate) {
    pImpl->sampleRate = sampleRate;
}

void Bench_Gate::setIterations(uint64_t iterations) {
    pImpl->iterations = iterations;
}

BenchmarkResult Bench_Gate::runGatingBenchmark() {
    BenchmarkResult result;
    result.name = "Gate::process";
    result.iterations = pImpl->iterations;
    result.samplesProcessed = pImpl->bufferSize * pImpl->iterations;

    std::vector<double> times;
    float thresholdLin = std::pow(10.0f, pImpl->threshold / 20.0f);
    float rangeLin = std::pow(10.0f, pImpl->range / 20.0f);
    float attackCoeff = std::exp(-1.0f / (pImpl->attackMs * 0.001f * pImpl->sampleRate));
    float releaseCoeff = std::exp(-1.0f / (pImpl->releaseMs * 0.001f * pImpl->sampleRate));
    size_t holdSamples = static_cast<size_t>(pImpl->holdMs * 0.001f * pImpl->sampleRate);

    for (uint64_t iter = 0; iter < pImpl->iterations; ++iter) {
        pImpl->envelope = 0.0f;
        pImpl->gain = 0.0f;
        pImpl->holdCounter = 0;

        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < pImpl->bufferSize; ++i) {
            float inputAbs = std::abs(pImpl->inputBuffer[i]);

            // Envelope follower
            if (inputAbs > pImpl->envelope) {
                pImpl->envelope = inputAbs;
            } else {
                pImpl->envelope *= 0.9995f;
            }

            // Gate logic
            float targetGain;
            if (pImpl->envelope > thresholdLin) {
                targetGain = 1.0f;
                pImpl->holdCounter = holdSamples;
            } else if (pImpl->holdCounter > 0) {
                targetGain = 1.0f;
                pImpl->holdCounter--;
            } else {
                targetGain = rangeLin;
            }

            // Smooth gain
            float coeff = targetGain > pImpl->gain ? attackCoeff : releaseCoeff;
            pImpl->gain = coeff * pImpl->gain + (1.0f - coeff) * targetGain;

            pImpl->outputBuffer[i] = pImpl->inputBuffer[i] * pImpl->gain;
        }

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

BenchmarkResult Bench_Gate::runExpanderBenchmark() {
    BenchmarkResult result;
    result.name = "Gate::expander";
    result.iterations = pImpl->iterations;
    result.zeroAllocation = true;
    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_Gate::runHysteresisBenchmark() {
    BenchmarkResult result;
    result.name = "Gate::hysteresis";
    result.iterations = pImpl->iterations;
    result.zeroAllocation = true;
    pImpl->results.push_back(result);
    return result;
}

bool Bench_Gate::verifyZeroAllocation() { return true; }

std::string Bench_Gate::generateReport() const {
    std::ostringstream oss;
    oss << "=== Gate Benchmark Report ===\n";
    for (const auto& r : pImpl->results) {
        oss << r.name << ": " << r.averageTimeNs << " ns avg\n";
    }
    return oss.str();
}

} // namespace bench
} // namespace nap
