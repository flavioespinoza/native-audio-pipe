#include "PanNode.h"
#include <cmath>
#include <string>

namespace nap {

class PanNode::Impl {
public:
    std::string nodeId;
    float pan = 0.0f;
    float targetPan = 0.0f;
    float leftGain = 1.0f;
    float rightGain = 1.0f;
    PanLaw panLaw = PanLaw::ConstantPower;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    bool bypassed = false;

    void updateGains() {
        float normalizedPan = (pan + 1.0f) * 0.5f; // 0 to 1

        switch (panLaw) {
            case PanLaw::Linear:
                leftGain = 1.0f - normalizedPan;
                rightGain = normalizedPan;
                break;
            case PanLaw::ConstantPower:
                leftGain = std::cos(normalizedPan * 3.14159265f * 0.5f);
                rightGain = std::sin(normalizedPan * 3.14159265f * 0.5f);
                break;
            case PanLaw::MinusFourPointFive:
                leftGain = std::sqrt((1.0f - normalizedPan) * 0.5f) * std::sqrt(2.0f);
                rightGain = std::sqrt(normalizedPan * 0.5f) * std::sqrt(2.0f);
                break;
        }
    }
};

PanNode::PanNode()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "PanNode_" + std::to_string(++instanceCounter);
    m_impl->updateGains();
}

PanNode::~PanNode() = default;

PanNode::PanNode(PanNode&&) noexcept = default;
PanNode& PanNode::operator=(PanNode&&) noexcept = default;

void PanNode::process(const float* inputBuffer, float* outputBuffer,
                       std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (m_impl->bypassed || numChannels < 2) {
        std::copy(inputBuffer, inputBuffer + numFrames * numChannels, outputBuffer);
        return;
    }

    for (std::uint32_t i = 0; i < numFrames; ++i) {
        outputBuffer[i * 2] = inputBuffer[i * 2] * m_impl->leftGain;
        outputBuffer[i * 2 + 1] = inputBuffer[i * 2 + 1] * m_impl->rightGain;
    }
}

void PanNode::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
}

void PanNode::reset()
{
    m_impl->pan = m_impl->targetPan;
    m_impl->updateGains();
}

std::string PanNode::getNodeId() const { return m_impl->nodeId; }
std::string PanNode::getTypeName() const { return "PanNode"; }
std::uint32_t PanNode::getNumInputChannels() const { return 2; }
std::uint32_t PanNode::getNumOutputChannels() const { return 2; }
bool PanNode::isBypassed() const { return m_impl->bypassed; }
void PanNode::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

void PanNode::setPan(float pan)
{
    m_impl->targetPan = std::max(-1.0f, std::min(1.0f, pan));
    m_impl->pan = m_impl->targetPan;
    m_impl->updateGains();
}

float PanNode::getPan() const { return m_impl->pan; }
void PanNode::setPanLaw(PanLaw law) { m_impl->panLaw = law; m_impl->updateGains(); }
PanNode::PanLaw PanNode::getPanLaw() const { return m_impl->panLaw; }

} // namespace nap
