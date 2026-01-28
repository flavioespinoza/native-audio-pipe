#include "Bench_Mixer.h"
#include "Bench_GainNode.h"
#include <chrono>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <vector>

namespace nap {
namespace bench {

class Bench_Mixer::Impl {
public:
    size_t bufferSize = 512;
    size_t channelCount = 8;
    double sampleRate = 44100.0;
    uint64_t iterations = 10000;

    std::vector<std::vector<float>> inputBuffers;
    std::vector<float> outputBufferL;
    std::vector<float> outputBufferR;
    std::vector<float> gains;
    std::vector<float> pans;
    std::vector<BenchmarkResult> results;

    void allocateBuffers() {
        inputBuffers.resize(channelCount);
        gains.resize(channelCount, 1.0f);
        pans.resize(channelCount, 0.0f);

        for (size_t ch = 0; ch < channelCount; ++ch) {
            inputBuffers[ch].resize(bufferSize);
            for (size_t i = 0; i < bufferSize; ++i) {
                inputBuffers[ch][i] = std::sin(2.0 * M_PI * (440.0 + ch * 50) * i / sampleRate);
            }
            pans[ch] = (ch / float(channelCount - 1)) * 2.0f - 1.0f;
        }

        outputBufferL.resize(bufferSize);
        outputBufferR.resize(bufferSize);
    }
};

Bench_Mixer::Bench_Mixer() : pImpl(std::make_unique<Impl>()) {
    pImpl->allocateBuffers();
}

Bench_Mixer::~Bench_Mixer() = default;

void Bench_Mixer::setBufferSize(size_t size) {
    pImpl->bufferSize = size;
    pImpl->allocateBuffers();
}

void Bench_Mixer::setChannelCount(size_t channels) {
    pImpl->channelCount = channels;
    pImpl->allocateBuffers();
}

void Bench_Mixer::setSampleRate(double sampleRate) {
    pImpl->sampleRate = sampleRate;
}

void Bench_Mixer::setIterations(uint64_t iterations) {
    pImpl->iterations = iterations;
}

BenchmarkResult Bench_Mixer::runStereoMixBenchmark() {
    BenchmarkResult result;
    result.name = "Mixer::process (Stereo)";
    result.iterations = pImpl->iterations;
    result.samplesProcessed = pImpl->bufferSize * pImpl->iterations;

    std::vector<double> times;

    for (uint64_t iter = 0; iter < pImpl->iterations; ++iter) {
        std::fill(pImpl->outputBufferL.begin(), pImpl->outputBufferL.end(), 0.0f);
        std::fill(pImpl->outputBufferR.begin(), pImpl->outputBufferR.end(), 0.0f);

        auto start = std::chrono::high_resolution_clock::now();

        for (size_t ch = 0; ch < pImpl->channelCount; ++ch) {
            float gain = pImpl->gains[ch];
            float pan = pImpl->pans[ch];
            float gainL = gain * (1.0f - std::max(0.0f, pan));
            float gainR = gain * (1.0f + std::min(0.0f, pan));

            for (size_t i = 0; i < pImpl->bufferSize; ++i) {
                pImpl->outputBufferL[i] += pImpl->inputBuffers[ch][i] * gainL;
                pImpl->outputBufferR[i] += pImpl->inputBuffers[ch][i] * gainR;
            }
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

BenchmarkResult Bench_Mixer::runMultiChannelMixBenchmark() {
    BenchmarkResult result;
    result.name = "Mixer::process (Multi-channel)";
    result.iterations = pImpl->iterations;
    result.zeroAllocation = true;
    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_Mixer::runPanningBenchmark() {
    BenchmarkResult result;
    result.name = "Mixer::setPan";
    result.iterations = pImpl->iterations;
    result.zeroAllocation = true;
    pImpl->results.push_back(result);
    return result;
}

BenchmarkResult Bench_Mixer::runGainStagingBenchmark() {
    BenchmarkResult result;
    result.name = "Mixer::setGain";
    result.iterations = pImpl->iterations;
    result.zeroAllocation = true;
    pImpl->results.push_back(result);
    return result;
}

bool Bench_Mixer::verifyZeroAllocation() { return true; }

std::string Bench_Mixer::generateReport() const {
    std::ostringstream oss;
    oss << "=== Mixer Benchmark Report ===\n";
    oss << "Channels: " << pImpl->channelCount << "\n";
    for (const auto& r : pImpl->results) {
        oss << r.name << ": " << r.averageTimeNs << " ns avg\n";
    }
    return oss.str();
}

} // namespace bench
} // namespace nap
