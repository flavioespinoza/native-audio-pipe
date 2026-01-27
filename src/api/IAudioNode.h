#ifndef NAP_IAUDIONODE_H
#define NAP_IAUDIONODE_H

#include <cstdint>
#include <string>
#include <vector>

namespace nap {

/**
 * @brief Base interface for all audio processing nodes in the graph.
 *
 * IAudioNode defines the fundamental contract that all audio nodes must implement.
 * This includes processing audio buffers, managing connections, and lifecycle management.
 */
class IAudioNode {
public:
    virtual ~IAudioNode() = default;

    /**
     * @brief Process audio data for one block.
     * @param inputBuffer Pointer to input audio samples
     * @param outputBuffer Pointer to output audio samples
     * @param numFrames Number of frames to process
     * @param numChannels Number of audio channels
     */
    virtual void process(const float* inputBuffer,
                         float* outputBuffer,
                         std::uint32_t numFrames,
                         std::uint32_t numChannels) = 0;

    /**
     * @brief Prepare the node for processing.
     * @param sampleRate The sample rate of the audio stream
     * @param blockSize The maximum block size for processing
     */
    virtual void prepare(double sampleRate, std::uint32_t blockSize) = 0;

    /**
     * @brief Reset the node's internal state.
     */
    virtual void reset() = 0;

    /**
     * @brief Get the unique identifier for this node instance.
     * @return Node ID as a string
     */
    virtual std::string getNodeId() const = 0;

    /**
     * @brief Get the type name of this node.
     * @return Type name as a string
     */
    virtual std::string getTypeName() const = 0;

    /**
     * @brief Get the number of input channels this node accepts.
     * @return Number of input channels
     */
    virtual std::uint32_t getNumInputChannels() const = 0;

    /**
     * @brief Get the number of output channels this node produces.
     * @return Number of output channels
     */
    virtual std::uint32_t getNumOutputChannels() const = 0;

    /**
     * @brief Check if this node is currently bypassed.
     * @return True if bypassed, false otherwise
     */
    virtual bool isBypassed() const = 0;

    /**
     * @brief Set the bypass state of this node.
     * @param bypassed True to bypass, false to enable processing
     */
    virtual void setBypassed(bool bypassed) = 0;
};

} // namespace nap

#endif // NAP_IAUDIONODE_H
