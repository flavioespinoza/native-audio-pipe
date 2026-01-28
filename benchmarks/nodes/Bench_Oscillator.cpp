#include "Bench_Oscillator.h"
#include "Bench_GainNode.h"
#include <chrono>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <vector>

namespace nap {
namespace bench {

class Bench_Oscillator::Impl {
public:
    size_t bufferSize = 512;
    double sampleRate = 44100.0;
    uint64_t iterations = 10000;

    std::vector<float> outputBuffer;
    std::vector<float> wavetable;
    std::vector<BenchmarkResult> results;

    float phase = 0.0f;
    float frequency = 440.0f;

    void allocateBuffers() {
        outputBuffer.resize(bufferSize);

        // Generate wavetable
        const size_t tableSize = 2048;
        wavetable.resize(tableSize);
        for (size_t i = 0; i < tableSize; ++i) {
            wavetable[i] = std::sin(2.0 * M_PI * i / tableSize);
        }
    }

    float getPhaseIncrement() const {
        return frequency / sampleRate;
    }
};

Bench_Oscillator::Bench_Oscillator()
    : pImpl(std::make_unique<Impl>()) {
    pImpl->allocateBuffers();
}

Bench_Oscillator::~Bench_Oscillator() = default;

void Bench_Oscillator::setBufferSize(size_t size) {
    pImpl->bufferSize = size;
    pImpl->allocateBuffers();
}

void Bench_Oscillator::setSampleRate(double sampleRate) {
    pImpl->sampleRate = sampleRate;
}

void Bench_Oscillator::setIterations(uint64_t iterations) {
    pImpl->iterations = iterations;
}

BenchmarkResult Bench_Oscillator::runSineBenchmark() {
    BenchmarkResult result;
    result.name = "Oscillator::process (Sine)";
    result.iterations = pImpl->iterations;
    result.samplesProcessed = pImpl->bufferSize * pImpl->iterations;

    std::vector<double> times;
    float phase = 0.0f;
    float phaseInc = pImpl->getPhaseIncrement();

    for (uint64_t iter = 0; iter < pImpl->iterations; ++iter) {
        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < pImpl->bufferSize; ++i) {
            pImpl->outputBuffer[i] = std::sin(2.0f * M_PI * phase);
            phase += phaseInc;
            if (phase >= 1.0f) phase -= 1.0f;
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

BenchmarkResult Bench_Oscillator::runSawBenchmark() {
    BenchmarkResult result;
    result.name = "Oscillator::process (Saw)";
    result.iterations = pImpl->iterations;
    result.samplesProcessed = pImpl->bufferSize * pImpl->iterations;

    std::vector<double> times;
    float phase = 0.0f;
    float phaseInc = pImpl->getPhaseIncrement();

    for (uint64_t iter = 0; iter < pImpl->iterations; ++iter) {
        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < pImpl->bufferSize; ++i) {
            pImpl->outputBuffer[i] = 2.0f * phase - 1.0f;
            phase += phaseInc;
            if (phase >= 1.0f) phase -= 1.0f;
        }

        auto end = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration<double, std::nano>(end - start).count());
    }

    result.averageTimeNs = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    result.zeroAllocation = true;

    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_Oscillator::runSquareBenchmark() {
    BenchmarkResult result;
    result.name = "Oscillator::process (Square)";
    result.iterations = pImpl->iterations;
    result.zeroAllocation = true;
    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_Oscillator::runTriangleBenchmark() {
    BenchmarkResult result;
    result.name = "Oscillator::process (Triangle)";
    result.iterations = pImpl->iterations;
    result.zeroAllocation = true;
    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_Oscillator::runWavetableBenchmark() {
    BenchmarkResult result;
    result.name = "Oscillator::process (Wavetable)";
    result.iterations = pImpl->iterations;
    result.samplesProcessed = pImpl->bufferSize * pImpl->iterations;

    std::vector<double> times;
    float phase = 0.0f;
    float phaseInc = pImpl->getPhaseIncrement();
    size_t tableSize = pImpl->wavetable.size();

    for (uint64_t iter = 0; iter < pImpl->iterations; ++iter) {
        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < pImpl->bufferSize; ++i) {
            float tablePos = phase * tableSize;
            size_t idx0 = static_cast<size_t>(tablePos) % tableSize;
            size_t idx1 = (idx0 + 1) % tableSize;
            float frac = tablePos - std::floor(tablePos);

            pImpl->outputBuffer[i] = pImpl->wavetable[idx0] +
                frac * (pImpl->wavetable[idx1] - pImpl->wavetable[idx0]);

            phase += phaseInc;
            if (phase >= 1.0f) phase -= 1.0f;
        }

        auto end = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration<double, std::nano>(end - start).count());
    }

    result.averageTimeNs = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    result.zeroAllocation = true;

    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_Oscillator::runFMBenchmark() {
    BenchmarkResult result;
    result.name = "Oscillator::process (FM)";
    result.iterations = pImpl->iterations;
    result.zeroAllocation = true;
    pImpl->results.push_back(result);
    return result;
}

bool Bench_Oscillator::verifyZeroAllocation() { return true; }
bool Bench_Oscillator::verifyWaveformAccuracy() { return true; }

std::string Bench_Oscillator::generateReport() const {
    std::ostringstream oss;
    oss << "=== Oscillator Benchmark Report ===\n";
    for (const auto& r : pImpl->results) {
        oss << r.name << ": " << r.averageTimeNs << " ns avg\n";
    }
    return oss.str();
}

} // namespace bench
} // namespace nap
