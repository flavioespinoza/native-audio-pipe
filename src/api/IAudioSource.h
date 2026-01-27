#ifndef NAP_IAUDIOSOURCE_H
#define NAP_IAUDIOSOURCE_H

#include <cstdint>

namespace nap {

/**
 * @brief Interface for audio source nodes that generate or provide audio data.
 *
 * IAudioSource extends the basic node concept for nodes that originate audio,
 * such as oscillators, file readers, or input device wrappers.
 */
class IAudioSource {
public:
    virtual ~IAudioSource() = default;

    /**
     * @brief Generate audio samples into the output buffer.
     * @param outputBuffer Pointer to output audio samples
     * @param numFrames Number of frames to generate
     * @param numChannels Number of audio channels
     */
    virtual void generate(float* outputBuffer,
                          std::uint32_t numFrames,
                          std::uint32_t numChannels) = 0;

    /**
     * @brief Check if this source has more data available.
     * @return True if more data is available, false if exhausted
     */
    virtual bool hasMoreData() const = 0;

    /**
     * @brief Get the total duration of this source in samples.
     * @return Duration in samples, or 0 if infinite/unknown
     */
    virtual std::uint64_t getTotalSamples() const = 0;

    /**
     * @brief Get the current playback position in samples.
     * @return Current position in samples
     */
    virtual std::uint64_t getCurrentPosition() const = 0;

    /**
     * @brief Seek to a specific position in the source.
     * @param positionSamples Target position in samples
     * @return True if seek was successful, false otherwise
     */
    virtual bool seek(std::uint64_t positionSamples) = 0;

    /**
     * @brief Check if this source supports seeking.
     * @return True if seekable, false otherwise
     */
    virtual bool isSeekable() const = 0;

    /**
     * @brief Start the source (begin generating/playing).
     */
    virtual void start() = 0;

    /**
     * @brief Stop the source (cease generating/playing).
     */
    virtual void stop() = 0;

    /**
     * @brief Check if the source is currently active.
     * @return True if active, false otherwise
     */
    virtual bool isActive() const = 0;
};

} // namespace nap

#endif // NAP_IAUDIOSOURCE_H
