#ifndef NAP_AUDIOBLOCKALLOCATOR_H
#define NAP_AUDIOBLOCKALLOCATOR_H

#include <cstdint>
#include <memory>

namespace nap {

/**
 * @brief Pre-allocated memory pool for audio processing blocks.
 *
 * AudioBlockAllocator provides fast, allocation-free memory management
 * for audio buffers during real-time processing.
 */
class AudioBlockAllocator {
public:
    /**
     * @brief Construct an allocator with specified block parameters.
     * @param blockSize Number of samples per block
     * @param numChannels Number of channels per block
     * @param numBlocks Number of blocks to pre-allocate
     */
    AudioBlockAllocator(std::uint32_t blockSize, std::uint32_t numChannels, std::uint32_t numBlocks);
    ~AudioBlockAllocator();

    AudioBlockAllocator(const AudioBlockAllocator&) = delete;
    AudioBlockAllocator& operator=(const AudioBlockAllocator&) = delete;
    AudioBlockAllocator(AudioBlockAllocator&&) noexcept;
    AudioBlockAllocator& operator=(AudioBlockAllocator&&) noexcept;

    /**
     * @brief Allocate a block of audio memory.
     * @return Pointer to allocated memory, or nullptr if pool exhausted
     */
    float* allocate();

    /**
     * @brief Return a block to the pool.
     * @param block Pointer to the block to return
     */
    void deallocate(float* block);

    /**
     * @brief Get the block size in samples.
     * @return Block size
     */
    std::uint32_t getBlockSize() const;

    /**
     * @brief Get the number of channels per block.
     * @return Number of channels
     */
    std::uint32_t getNumChannels() const;

    /**
     * @brief Get the total number of pre-allocated blocks.
     * @return Total block count
     */
    std::uint32_t getTotalBlocks() const;

    /**
     * @brief Get the number of currently available blocks.
     * @return Available block count
     */
    std::uint32_t getAvailableBlocks() const;

    /**
     * @brief Get the total memory size in bytes.
     * @return Total memory allocated
     */
    std::size_t getTotalMemorySize() const;

    /**
     * @brief Reset the allocator, returning all blocks to the pool.
     */
    void reset();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

/**
 * @brief RAII wrapper for AudioBlockAllocator blocks.
 */
class ScopedAudioBlock {
public:
    ScopedAudioBlock(AudioBlockAllocator& allocator);
    ~ScopedAudioBlock();

    ScopedAudioBlock(const ScopedAudioBlock&) = delete;
    ScopedAudioBlock& operator=(const ScopedAudioBlock&) = delete;
    ScopedAudioBlock(ScopedAudioBlock&& other) noexcept;
    ScopedAudioBlock& operator=(ScopedAudioBlock&& other) noexcept;

    float* get() const;
    float* operator->() const;
    float& operator[](std::size_t index);
    const float& operator[](std::size_t index) const;
    explicit operator bool() const;

private:
    AudioBlockAllocator* m_allocator;
    float* m_block;
};

} // namespace nap

#endif // NAP_AUDIOBLOCKALLOCATOR_H
