#include "WhiteNoise.h"
#include <random>
#include <string>

namespace nap {

class WhiteNoise::Impl {
public:
    std::string nodeId;
    float amplitude = 1.0f;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;
    bool active = false;
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist{-1.0f, 1.0f};
};

WhiteNoise::WhiteNoise()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "WhiteNoise_" + std::to_string(++instanceCounter);
    m_impl->rng.seed(std::random_device{}());
}

WhiteNoise::~WhiteNoise() = default;

WhiteNoise::WhiteNoise(WhiteNoise&&) noexcept = default;
WhiteNoise& WhiteNoise::operator=(WhiteNoise&&) noexcept = default;

void WhiteNoise::process(const float* /*inputBuffer*/, float* outputBuffer,
                          std::uint32_t numFrames, std::uint32_t numChannels)
{
    generate(outputBuffer, numFrames, numChannels);
}

void WhiteNoise::generate(float* outputBuffer, std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (!m_impl->active || m_impl->bypassed) {
        std::fill(outputBuffer, outputBuffer + numFrames * numChannels, 0.0f);
        return;
    }

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        float sample = m_impl->dist(m_impl->rng) * m_impl->amplitude;
        for (std::uint32_t ch = 0; ch < numChannels; ++ch) {
            outputBuffer[i * numChannels + ch] = sample;
        }
    }
}

void WhiteNoise::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
}

void WhiteNoise::reset()
{
    m_impl->rng.seed(std::random_device{}());
}

std::string WhiteNoise::getNodeId() const { return m_impl->nodeId; }
std::string WhiteNoise::getTypeName() const { return "WhiteNoise"; }
std::uint32_t WhiteNoise::getNumInputChannels() const { return 0; }
std::uint32_t WhiteNoise::getNumOutputChannels() const { return 2; }
bool WhiteNoise::isBypassed() const { return m_impl->bypassed; }
void WhiteNoise::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

bool WhiteNoise::hasMoreData() const { return m_impl->active; }
std::uint64_t WhiteNoise::getTotalSamples() const { return 0; }
std::uint64_t WhiteNoise::getCurrentPosition() const { return 0; }
bool WhiteNoise::seek(std::uint64_t /*positionSamples*/) { return false; }
bool WhiteNoise::isSeekable() const { return false; }
void WhiteNoise::start() { m_impl->active = true; }
void WhiteNoise::stop() { m_impl->active = false; }
bool WhiteNoise::isActive() const { return m_impl->active; }

void WhiteNoise::setAmplitude(float amplitude) { m_impl->amplitude = amplitude; }
float WhiteNoise::getAmplitude() const { return m_impl->amplitude; }
void WhiteNoise::setSeed(std::uint32_t seed) { m_impl->rng.seed(seed); }

} // namespace nap
