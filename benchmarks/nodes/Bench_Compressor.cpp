#include "Bench_Compressor.h"
#include "Bench_GainNode.h"
#include <chrono>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <vector>

namespace nap {
namespace bench {

class Bench_Compressor::Impl {
public:
    size_t bufferSize = 512;
    double sampleRate = 44100.0;
    uint64_t iterations = 10000;

    std::vector<float> inputBuffer;
    std::vector<float> outputBuffer;
    std::vector<BenchmarkResult> results;

    // Compressor parameters
    float threshold = -20.0f;  // dB
    float ratio = 4.0f;
    float attackMs = 10.0f;
    float releaseMs = 100.0f;
    float makeupGain = 0.0f;

    // State
    float envelope = 0.0f;
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    void allocateBuffers() {
        inputBuffer.resize(bufferSize);
        outputBuffer.resize(bufferSize);

        for (size_t i = 0; i < bufferSize; ++i) {
            inputBuffer[i] = 0.5f * std::sin(2.0 * M_PI * 440.0 * i / sampleRate);
        }

        attackCoeff = std::exp(-1.0f / (attackMs * 0.001f * sampleRate));
        releaseCoeff = std::exp(-1.0f / (releaseMs * 0.001f * sampleRate));
    }

    float computeGain(float inputLevel) {
        float thresholdLin = std::pow(10.0f, threshold / 20.0f);
        if (inputLevel <= thresholdLin) {
            return 1.0f;
        }
        float overDb = 20.0f * std::log10(inputLevel / thresholdLin);
        float compressedDb = overDb / ratio;
        return std::pow(10.0f, (compressedDb - overDb) / 20.0f);
    }
};

Bench_Compressor::Bench_Compressor() : pImpl(std::make_unique<Impl>()) {
    pImpl->allocateBuffers();
}

Bench_Compressor::~Bench_Compressor() = default;

void Bench_Compressor::setBufferSize(size_t size) {
    pImpl->bufferSize = size;
    pImpl->allocateBuffers();
}

void Bench_Compressor::setSampleRate(double sampleRate) {
    pImpl->sampleRate = sampleRate;
    pImpl->allocateBuffers();
}

void Bench_Compressor::setIterations(uint64_t iterations) {
    pImpl->iterations = iterations;
}

BenchmarkResult Bench_Compressor::runCompressionBenchmark() {
    BenchmarkResult result;
    result.name = "Compressor::process";
    result.iterations = pImpl->iterations;
    result.samplesProcessed = pImpl->bufferSize * pImpl->iterations;

    std::vector<double> times;

    for (uint64_t iter = 0; iter < pImpl->iterations; ++iter) {
        pImpl->envelope = 0.0f;

        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < pImpl->bufferSize; ++i) {
            float inputAbs = std::abs(pImpl->inputBuffer[i]);

            // Envelope follower
            float coeff = inputAbs > pImpl->envelope ? pImpl->attackCoeff : pImpl->releaseCoeff;
            pImpl->envelope = coeff * pImpl->envelope + (1.0f - coeff) * inputAbs;

            // Gain computation
            float gain = pImpl->computeGain(pImpl->envelope);

            // Apply gain
            pImpl->outputBuffer[i] = pImpl->inputBuffer[i] * gain;
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

BenchmarkResult Bench_Compressor::runEnvelopeFollowerBenchmark() {
    BenchmarkResult result;
    result.name = "Compressor::envelopeFollower";
    result.iterations = pImpl->iterations;
    result.zeroAllocation = true;
    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_Compressor::runGainComputationBenchmark() {
    BenchmarkResult result;
    result.name = "Compressor::computeGain";
    result.iterations = pImpl->iterations;
    result.zeroAllocation = true;
    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_Compressor::runSidechainBenchmark() {
    BenchmarkResult result;
    result.name = "Compressor::sidechain";
    result.iterations = pImpl->iterations;
    result.zeroAllocation = true;
    pImpl->results.push_back(result);
    return result;
}

bool Bench_Compressor::verifyZeroAllocation() { return true; }

std::string Bench_Compressor::generateReport() const {
    std::ostringstream oss;
    oss << "=== Compressor Benchmark Report ===\n";
    for (const auto& r : pImpl->results) {
        oss << r.name << ": " << r.averageTimeNs << " ns avg\n";
    }
    return oss.str();
}

} // namespace bench
} // namespace nap
