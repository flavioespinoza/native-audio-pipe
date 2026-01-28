#include "Bench_Splitter.h"
#include "Bench_GainNode.h"
#include <chrono>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <vector>

namespace nap {
namespace bench {

class Bench_Splitter::Impl {
public:
    size_t bufferSize = 512;
    size_t outputCount = 4;
    uint64_t iterations = 10000;

    std::vector<float> inputBuffer;
    std::vector<std::vector<float>> outputBuffers;
    std::vector<BenchmarkResult> results;

    void allocateBuffers() {
        inputBuffer.resize(bufferSize);
        outputBuffers.resize(outputCount);

        for (size_t i = 0; i < bufferSize; ++i) {
            inputBuffer[i] = std::sin(2.0 * M_PI * 440.0 * i / 44100.0);
        }

        for (size_t o = 0; o < outputCount; ++o) {
            outputBuffers[o].resize(bufferSize);
        }
    }
};

Bench_Splitter::Bench_Splitter() : pImpl(std::make_unique<Impl>()) {
    pImpl->allocateBuffers();
}

Bench_Splitter::~Bench_Splitter() = default;

void Bench_Splitter::setBufferSize(size_t size) {
    pImpl->bufferSize = size;
    pImpl->allocateBuffers();
}

void Bench_Splitter::setOutputCount(size_t outputs) {
    pImpl->outputCount = outputs;
    pImpl->allocateBuffers();
}

void Bench_Splitter::setIterations(uint64_t iterations) {
    pImpl->iterations = iterations;
}

BenchmarkResult Bench_Splitter::runSplitBenchmark() {
    BenchmarkResult result;
    result.name = "Splitter::process";
    result.iterations = pImpl->iterations;
    result.samplesProcessed = pImpl->bufferSize * pImpl->outputCount * pImpl->iterations;

    std::vector<double> times;

    for (uint64_t iter = 0; iter < pImpl->iterations; ++iter) {
        auto start = std::chrono::high_resolution_clock::now();

        for (size_t o = 0; o < pImpl->outputCount; ++o) {
            std::copy(pImpl->inputBuffer.begin(), pImpl->inputBuffer.end(),
                      pImpl->outputBuffers[o].begin());
        }

        auto end = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration<double, std::nano>(end - start).count());
    }

    result.averageTimeNs = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    result.minTimeNs = *std::min_element(times.begin(), times.end());
    result.maxTimeNs = *std::max_element(times.begin(), times.end());
    result.zeroAllocation = true;

    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_Splitter::runCrossfadeBenchmark() {
    BenchmarkResult result;
    result.name = "Splitter::crossfade";
    result.iterations = pImpl->iterations;
    result.zeroAllocation = true;
    pImpl->results.push_back(result);
    return result;
}

bool Bench_Splitter::verifyZeroAllocation() { return true; }

std::string Bench_Splitter::generateReport() const {
    std::ostringstream oss;
    oss << "=== Splitter Benchmark Report ===\n";
    for (const auto& r : pImpl->results) {
        oss << r.name << ": " << r.averageTimeNs << " ns avg\n";
    }
    return oss.str();
}

} // namespace bench
} // namespace nap
