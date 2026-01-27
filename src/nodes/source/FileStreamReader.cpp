#include "FileStreamReader.h"
#include <algorithm>

namespace nap {

class FileStreamReader::Impl {
public:
    std::string nodeId;
    std::string filePath;
    double sampleRate = 44100.0;
    std::uint32_t blockSize = 512;
    double fileSampleRate = 0.0;
    std::uint32_t fileChannels = 0;
    std::uint64_t totalSamples = 0;
    std::uint64_t currentPosition = 0;
    bool bypassed = false;
    bool active = false;
    bool fileLoaded = false;
    bool looping = false;
};

FileStreamReader::FileStreamReader()
    : m_impl(std::make_unique<Impl>())
{
    static int instanceCounter = 0;
    m_impl->nodeId = "FileStreamReader_" + std::to_string(++instanceCounter);
}

FileStreamReader::~FileStreamReader() = default;

FileStreamReader::FileStreamReader(FileStreamReader&&) noexcept = default;
FileStreamReader& FileStreamReader::operator=(FileStreamReader&&) noexcept = default;

void FileStreamReader::process(const float* /*inputBuffer*/, float* outputBuffer,
                                std::uint32_t numFrames, std::uint32_t numChannels)
{
    generate(outputBuffer, numFrames, numChannels);
}

void FileStreamReader::generate(float* outputBuffer, std::uint32_t numFrames, std::uint32_t numChannels)
{
    if (!m_impl->active || m_impl->bypassed || !m_impl->fileLoaded) {
        std::fill(outputBuffer, outputBuffer + numFrames * numChannels, 0.0f);
        return;
    }

    // TODO: Implement actual file streaming
    std::fill(outputBuffer, outputBuffer + numFrames * numChannels, 0.0f);
}

void FileStreamReader::prepare(double sampleRate, std::uint32_t blockSize)
{
    m_impl->sampleRate = sampleRate;
    m_impl->blockSize = blockSize;
}

void FileStreamReader::reset()
{
    m_impl->currentPosition = 0;
}

std::string FileStreamReader::getNodeId() const { return m_impl->nodeId; }
std::string FileStreamReader::getTypeName() const { return "FileStreamReader"; }
std::uint32_t FileStreamReader::getNumInputChannels() const { return 0; }
std::uint32_t FileStreamReader::getNumOutputChannels() const { return 2; }
bool FileStreamReader::isBypassed() const { return m_impl->bypassed; }
void FileStreamReader::setBypassed(bool bypassed) { m_impl->bypassed = bypassed; }

bool FileStreamReader::hasMoreData() const
{
    return m_impl->looping || m_impl->currentPosition < m_impl->totalSamples;
}

std::uint64_t FileStreamReader::getTotalSamples() const { return m_impl->totalSamples; }
std::uint64_t FileStreamReader::getCurrentPosition() const { return m_impl->currentPosition; }

bool FileStreamReader::seek(std::uint64_t positionSamples)
{
    if (positionSamples <= m_impl->totalSamples) {
        m_impl->currentPosition = positionSamples;
        return true;
    }
    return false;
}

bool FileStreamReader::isSeekable() const { return m_impl->fileLoaded; }
void FileStreamReader::start() { m_impl->active = true; }
void FileStreamReader::stop() { m_impl->active = false; }
bool FileStreamReader::isActive() const { return m_impl->active; }

bool FileStreamReader::loadFile(const std::string& filePath)
{
    // TODO: Implement actual file loading
    m_impl->filePath = filePath;
    m_impl->fileLoaded = true;
    return true;
}

void FileStreamReader::unloadFile()
{
    m_impl->filePath.clear();
    m_impl->fileLoaded = false;
    m_impl->totalSamples = 0;
    m_impl->currentPosition = 0;
}

bool FileStreamReader::isFileLoaded() const { return m_impl->fileLoaded; }
std::string FileStreamReader::getFilePath() const { return m_impl->filePath; }
double FileStreamReader::getFileSampleRate() const { return m_impl->fileSampleRate; }
std::uint32_t FileStreamReader::getFileChannels() const { return m_impl->fileChannels; }
void FileStreamReader::setLooping(bool loop) { m_impl->looping = loop; }
bool FileStreamReader::isLooping() const { return m_impl->looping; }

} // namespace nap
