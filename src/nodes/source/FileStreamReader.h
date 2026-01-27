#ifndef NAP_FILESTREAMREADER_H
#define NAP_FILESTREAMREADER_H

#include "../../api/IAudioNode.h"
#include "../../api/IAudioSource.h"
#include <memory>
#include <string>

namespace nap {

/**
 * @brief Audio source node that streams audio from a file.
 */
class FileStreamReader : public IAudioNode, public IAudioSource {
public:
    FileStreamReader();
    ~FileStreamReader() override;

    FileStreamReader(const FileStreamReader&) = delete;
    FileStreamReader& operator=(const FileStreamReader&) = delete;
    FileStreamReader(FileStreamReader&&) noexcept;
    FileStreamReader& operator=(FileStreamReader&&) noexcept;

    // IAudioNode interface
    void process(const float* inputBuffer, float* outputBuffer,
                 std::uint32_t numFrames, std::uint32_t numChannels) override;
    void prepare(double sampleRate, std::uint32_t blockSize) override;
    void reset() override;
    std::string getNodeId() const override;
    std::string getTypeName() const override;
    std::uint32_t getNumInputChannels() const override;
    std::uint32_t getNumOutputChannels() const override;
    bool isBypassed() const override;
    void setBypassed(bool bypassed) override;

    // IAudioSource interface
    void generate(float* outputBuffer, std::uint32_t numFrames, std::uint32_t numChannels) override;
    bool hasMoreData() const override;
    std::uint64_t getTotalSamples() const override;
    std::uint64_t getCurrentPosition() const override;
    bool seek(std::uint64_t positionSamples) override;
    bool isSeekable() const override;
    void start() override;
    void stop() override;
    bool isActive() const override;

    // FileStreamReader specific
    bool loadFile(const std::string& filePath);
    void unloadFile();
    bool isFileLoaded() const;
    std::string getFilePath() const;
    double getFileSampleRate() const;
    std::uint32_t getFileChannels() const;
    void setLooping(bool loop);
    bool isLooping() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_FILESTREAMREADER_H
