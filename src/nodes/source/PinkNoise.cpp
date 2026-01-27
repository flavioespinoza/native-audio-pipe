#include "PinkNoise.h"
#include <array>
#include <random>
#include <string>

namespace nap {

class PinkNoise::Impl {
public:
    std::string nodeId;
    float amplitude = 1.0f;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;
    bool active = false;
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist{-1.0f, 1.0f};

    // Voss-McCartney algorithm state
    std::array<float, 16> octaves{};
    int counter = 0;
};

PinkNoise::PinkNoise()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "PinkNoise_" + std::to_string(++instanceCounter);
    m_impl->rng.seed(std::random_device{}());
    m_impl->octaves.fill(0.0f);
}

PinkNoise::~PinkNoise() = default;

PinkNoise::PinkNoise(PinkNoise&&) noexcept = default;
PinkNoise& PinkNoise::operator=(PinkNoise&&) noexcept = default;

void PinkNoise::process(const float* /*inputBuffer*/, float* outputBuffer,
                         std::uint32_t numFrames, std::uint32_t numChannels)
{
    generate(outputBuffer, numFrames, numChannels);
}

void PinkNoise::generate(float* outputBuffer, std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (!m_impl->active || m_impl->bypassed) {
        std::fill(outputBuffer, outputBuffer + numFrames * numChannels, 0.0f);
        return;
    }

    constexpr int numOctaves = 16;

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        // Voss-McCartney algorithm for pink noise
        int k = m_impl->counter;
        m_impl->counter++;

        float sum = 0.0f;
        for (int j = 0; j < numOctaves; ++j) {
            if ((k & (1 << j)) == 0) {
                m_impl->octaves[j] = m_impl->dist(m_impl->rng);
            }
            sum += m_impl->octaves[j];
        }

        float sample = (sum / numOctaves) * m_impl->amplitude;

        for (std::uint32_t ch = 0; ch < numChannels; ++ch) {
            outputBuffer[i * numChannels + ch] = sample;
        }
    }
}

void PinkNoise::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
}

void PinkNoise::reset()
{
    m_impl->rng.seed(std::random_device{}());
    m_impl->octaves.fill(0.0f);
    m_impl->counter = 0;
}

std::string PinkNoise::getNodeId() const { return m_impl->nodeId; }
std::string PinkNoise::getTypeName() const { return "PinkNoise"; }
std::uint32_t PinkNoise::getNumInputChannels() const { return 0; }
std::uint32_t PinkNoise::getNumOutputChannels() const { return 2; }
bool PinkNoise::isBypassed() const { return m_impl->bypassed; }
void PinkNoise::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

bool PinkNoise::hasMoreData() const { return m_impl->active; }
std::uint64_t PinkNoise::getTotalSamples() const { return 0; }
std::uint64_t PinkNoise::getCurrentPosition() const { return 0; }
bool PinkNoise::seek(std::uint64_t /*positionSamples*/) { return false; }
bool PinkNoise::isSeekable() const { return false; }
void PinkNoise::start() { m_impl->active = true; }
void PinkNoise::stop() { m_impl->active = false; }
bool PinkNoise::isActive() const { return m_impl->active; }

void PinkNoise::setAmplitude(float amplitude) { m_impl->amplitude = amplitude; }
float PinkNoise::getAmplitude() const { return m_impl->amplitude; }
void PinkNoise::setSeed(std::uint32_t seed) { m_impl->rng.seed(seed); }

} // namespace nap
