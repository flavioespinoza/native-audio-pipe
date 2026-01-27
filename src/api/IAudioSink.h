#ifndef NAP_IAUDIOSINK_H
#define NAP_IAUDIOSINK_H

#include <cstdint>

namespace nap {

/**
 * @brief Interface for audio sink nodes that consume or output audio data.
 *
 * IAudioSink extends the basic node concept for nodes that terminate audio flow,
 * such as output devices, file writers, or analysis endpoints.
 */
class IAudioSink {
public:
    virtual ~IAudioSink() = default;

    /**
     * @brief Consume audio samples from the input buffer.
     * @param inputBuffer Pointer to input audio samples
     * @param numFrames Number of frames to consume
     * @param numChannels Number of audio channels
     */
    virtual void consume(const float* inputBuffer,
                         std::uint32_t numFrames,
                         std::uint32_t numChannels) = 0;

    /**
     * @brief Check if this sink is ready to receive data.
     * @return True if ready, false otherwise
     */
    virtual bool isReady() const = 0;

    /**
     * @brief Get the number of samples currently buffered in the sink.
     * @return Number of buffered samples
     */
    virtual std::uint64_t getBufferedSamples() const = 0;

    /**
     * @brief Flush any buffered data in the sink.
     */
    virtual void flush() = 0;

    /**
     * @brief Get the latency introduced by this sink in samples.
     * @return Latency in samples
     */
    virtual std::uint32_t getLatencySamples() const = 0;

    /**
     * @brief Open the sink for receiving data.
     * @return True if opened successfully, false otherwise
     */
    virtual bool open() = 0;

    /**
     * @brief Close the sink.
     */
    virtual void close() = 0;

    /**
     * @brief Check if the sink is currently open.
     * @return True if open, false otherwise
     */
    virtual bool isOpen() const = 0;
};

} // namespace nap

#endif // NAP_IAUDIOSINK_H
