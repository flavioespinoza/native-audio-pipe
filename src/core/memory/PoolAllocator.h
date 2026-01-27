#ifndef NAP_POOLALLOCATOR_H
#define NAP_POOLALLOCATOR_H

#include <cstdint>
#include <memory>

namespace nap {

/**
 * @brief Generic fixed-size pool allocator for real-time audio processing.
 *
 * PoolAllocator provides O(1) allocation and deallocation of fixed-size
 * memory blocks, suitable for real-time audio processing where dynamic
 * allocation must be avoided.
 */
class PoolAllocator {
public:
    /**
     * @brief Construct a pool allocator.
     * @param blockSize Size of each block in bytes
     * @param numBlocks Number of blocks to pre-allocate
     * @param alignment Memory alignment in bytes (default 16 for SIMD)
     */
    PoolAllocator(std::size_t blockSize, std::size_t numBlocks, std::size_t alignment = 16);
    ~PoolAllocator();

    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;
    PoolAllocator(PoolAllocator&&) noexcept;
    PoolAllocator& operator=(PoolAllocator&&) noexcept;

    /**
     * @brief Allocate a block from the pool.
     * @return Pointer to allocated memory, or nullptr if pool exhausted
     */
    void* allocate();

    /**
     * @brief Return a block to the pool.
     * @param ptr Pointer to the block to return
     */
    void deallocate(void* ptr);

    /**
     * @brief Allocate and construct an object of type T.
     * @tparam T Type to construct
     * @tparam Args Constructor argument types
     * @param args Constructor arguments
     * @return Pointer to constructed object, or nullptr if allocation failed
     */
    template<typename T, typename... Args>
    T* construct(Args&&... args)
    {
        void* ptr = allocate();
        if (!ptr) {
            return nullptr;
        }
        return new (ptr) T(std::forward<Args>(args)...);
    }

    /**
     * @brief Destroy and deallocate an object.
     * @tparam T Type of object to destroy
     * @param ptr Pointer to the object
     */
    template<typename T>
    void destroy(T* ptr)
    {
        if (ptr) {
            ptr->~T();
            deallocate(ptr);
        }
    }

    /**
     * @brief Get the size of each block.
     * @return Block size in bytes
     */
    std::size_t getBlockSize() const;

    /**
     * @brief Get the total number of blocks.
     * @return Total block count
     */
    std::size_t getTotalBlocks() const;

    /**
     * @brief Get the number of available blocks.
     * @return Available block count
     */
    std::size_t getAvailableBlocks() const;

    /**
     * @brief Get the memory alignment.
     * @return Alignment in bytes
     */
    std::size_t getAlignment() const;

    /**
     * @brief Get total memory used by the pool.
     * @return Total memory in bytes
     */
    std::size_t getTotalMemorySize() const;

    /**
     * @brief Check if a pointer belongs to this pool.
     * @param ptr Pointer to check
     * @return True if the pointer is within this pool's memory
     */
    bool owns(const void* ptr) const;

    /**
     * @brief Reset the pool, making all blocks available again.
     */
    void reset();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace nap

#endif // NAP_POOLALLOCATOR_H
