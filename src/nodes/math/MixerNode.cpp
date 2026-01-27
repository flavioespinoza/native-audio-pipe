#include "MixerNode.h"
#include <algorithm>
#include <string>
#include <vector>

namespace nap {

class MixerNode::Impl {
public:
    explicit Impl(std::uint32_t numInputs)
        : numInputs(numInputs)
        , inputGains(numInputs, 1.0f)
        , inputMuted(numInputs, false)
    {
    }

    std::string nodeId;
    std::uint32_t numInputs;
    std::vector<float> inputGains;
    std::vector<bool> inputMuted;
    float masterGain = 1.0f;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;
};

MixerNode::MixerNode(std::uint32_t numInputs)
    : m_impl(std::make_unique<Impl>(numInputs))
{
    static int instanceCounter = 0;
    m_impl->nodeId = "MixerNode_" + std::to_string(++instanceCounter);
}

MixerNode::~MixerNode() = default;

MixerNode::MixerNode(MixerNode&&) noexcept = default;
MixerNode& MixerNode::operator=(MixerNode&&) noexcept = default;

void MixerNode::process(const float* inputBuffer, float* outputBuffer,
                         std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    std::fill(outputBuffer, outputBuffer + numFrames * numChannels, 0.0f);

    // TODO: Handle multiple input streams
    for (std::uint32_t i = 0; i < numFrames * numChannels; ++i) {
        outputBuffer[i] = inputBuffer[i] * m_impl->inputGains[0] * m_impl->masterGain;
    }
}

void MixerNode::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
}

void MixerNode::reset()
{
    std::fill(m_impl->inputGains.begin(), m_impl->inputGains.end(), 1.0f);
    std::fill(m_impl->inputMuted.begin(), m_impl->inputMuted.end(), false);
}

std::string MixerNode::getNodeId() const { return m_impl->nodeId; }
std::string MixerNode::getTypeName() const { return "MixerNode"; }
std::uint32_t MixerNode::getNumInputChannels() const { return m_impl->numInputs * 2; }
std::uint32_t MixerNode::getNumOutputChannels() const { return 2; }
bool MixerNode::isBypassed() const { return m_impl->bypassed; }
void MixerNode::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void MixerNode::setInputGain(std::uint32_t inputIndex, float gain)
{
    if (inputIndex < m_impl->numInputs) {
        m_impl->inputGains[inputIndex] = gain;
    }
}

float MixerNode::getInputGain(std::uint32_t inputIndex) const
{
    return inputIndex < m_impl->numInputs ? m_impl->inputGains[inputIndex] : 0.0f;
}

void MixerNode::setMasterGain(float gain) { m_impl->masterGain = gain; }
float MixerNode::getMasterGain() const { return m_impl->masterGain; }
std::uint32_t MixerNode::getNumInputs() const { return m_impl->numInputs; }

void MixerNode::muteInput(std::uint32_t inputIndex, bool mute)
{
    if (inputIndex < m_impl->numInputs) {
        m_impl->inputMuted[inputIndex] = mute;
    }
}

bool MixerNode::isInputMuted(std::uint32_t inputIndex) const
{
    return inputIndex < m_impl->numInputs ? m_impl->inputMuted[inputIndex] : true;
}

} // namespace nap
