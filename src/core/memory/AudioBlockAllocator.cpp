#include "AudioBlockAllocator.h"
#include <cstring>
#include <stack>
#include <vector>

namespace nap {

class AudioBlockAllocator::Impl {
public:
    Impl(std::uint32_t blockSize, std::uint32_t numChannels, std::uint32_t numBlocks)
        : blockSize(blockSize)
        , numChannels(numChannels)
        , numBlocks(numBlocks)
        , samplesPerBlock(blockSize * numChannels)
    {
        memory.resize(static_cast<std::size_t>(numBlocks) * samplesPerBlock);

        for (std::uint32_t i = 0; i < numBlocks; ++i) {
            freeBlocks.push(&memory[static_cast<std::size_t>(i) * samplesPerBlock]);
        }
    }

    std::vector<float> memory;
    std::stack<float*> freeBlocks;
    std::uint32_t blockSize;
    std::uint32_t numChannels;
    std::uint32_t numBlocks;
    std::uint32_t samplesPerBlock;
};

AudioBlockAllocator::AudioBlockAllocator(std::uint32_t blockSize, std::uint32_t numChannels,
                                          std::uint32_t numBlocks)
    : m_impl(std::make_unique<Impl>(blockSize, numChannels, numBlocks))
{
}

AudioBlockAllocator::~AudioBlockAllocator() = default;

AudioBlockAllocator::AudioBlockAllocator(AudioBlockAllocator&&) noexcept = default;
AudioBlockAllocator& AudioBlockAllocator::operator=(AudioBlockAllocator&&) noexcept = default;

float* AudioBlockAllocator::allocate()
{
    if (m_impl->freeBlocks.empty()) {
        return nullptr;
    }

    float* block = m_impl->freeBlocks.top();
    m_impl->freeBlocks.pop();

    std::memset(block, 0, m_impl->samplesPerBlock * sizeof(float));
    return block;
}

void AudioBlockAllocator::deallocate(float* block)
{
    if (block) {
        m_impl->freeBlocks.push(block);
    }
}

std::uint32_t AudioBlockAllocator::getBlockSize() const
{
    return m_impl->blockSize;
}

std::uint32_t AudioBlockAllocator::getNumChannels() const
{
    return m_impl->numChannels;
}

std::uint32_t AudioBlockAllocator::getTotalBlocks() const
{
    return m_impl->numBlocks;
}

std::uint32_t AudioBlockAllocator::getAvailableBlocks() const
{
    return static_cast<std::uint32_t>(m_impl->freeBlocks.size());
}

std::size_t AudioBlockAllocator::getTotalMemorySize() const
{
    return m_impl->memory.size() * sizeof(float);
}

void AudioBlockAllocator::reset()
{
    while (!m_impl->freeBlocks.empty()) {
        m_impl->freeBlocks.pop();
    }

    for (std::uint32_t i = 0; i < m_impl->numBlocks; ++i) {
        m_impl->freeBlocks.push(&m_impl->memory[static_cast<std::size_t>(i) * m_impl->samplesPerBlock]);
    }
}

// ScopedAudioBlock implementation

ScopedAudioBlock::ScopedAudioBlock(AudioBlockAllocator& allocator)
    : m_allocator(&allocator)
    , m_block(allocator.allocate())
{
}

ScopedAudioBlock::~ScopedAudioBlock()
{
    if (m_block && m_allocator) {
        m_allocator->deallocate(m_block);
    }
}

ScopedAudioBlock::ScopedAudioBlock(ScopedAudioBlock&& other) noexcept
    : m_allocator(other.m_allocator)
    , m_block(other.m_block)
{
    other.m_allocator = nullptr;
    other.m_block = nullptr;
}

ScopedAudioBlock& ScopedAudioBlock::operator=(ScopedAudioBlock&& other) noexcept
{
    if (this != &other) {
        if (m_block && m_allocator) {
            m_allocator->deallocate(m_block);
        }
        m_allocator = other.m_allocator;
        m_block = other.m_block;
        other.m_allocator = nullptr;
        other.m_block = nullptr;
    }
    return *this;
}

float* ScopedAudioBlock::get() const
{
    return m_block;
}

float* ScopedAudioBlock::operator->() const
{
    return m_block;
}

float& ScopedAudioBlock::operator[](std::size_t index)
{
    return m_block[index];
}

const float& ScopedAudioBlock::operator[](std::size_t index) const
{
    return m_block[index];
}

ScopedAudioBlock::operator bool() const
{
    return m_block != nullptr;
}

} // namespace nap
