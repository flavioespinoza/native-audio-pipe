#include "PoolAllocator.h"
#include <cstdlib>
#include <stack>
#include <vector>

namespace nap {

class PoolAllocator::Impl {
public:
    Impl(std::size_t blockSize, std::size_t numBlocks, std::size_t alignment)
        : blockSize(blockSize)
        , numBlocks(numBlocks)
        , alignment(alignment)
        , alignedBlockSize((blockSize + alignment - 1) & ~(alignment - 1))
    {
        totalMemorySize = alignedBlockSize * numBlocks + alignment;

#if defined(_MSC_VER)
        memory = static_cast<char*>(_aligned_malloc(totalMemorySize, alignment));
#else
        void* ptr = nullptr;
        if (posix_memalign(&ptr, alignment, totalMemorySize) == 0) {
            memory = static_cast<char*>(ptr);
        } else {
            memory = nullptr;
        }
#endif

        if (memory) {
            for (std::size_t i = 0; i < numBlocks; ++i) {
                freeBlocks.push(memory + i * alignedBlockSize);
            }
        }
    }

    ~Impl()
    {
        if (memory) {
#if defined(_MSC_VER)
            _aligned_free(memory);
#else
            free(memory);
#endif
        }
    }

    char* memory = nullptr;
    std::stack<void*> freeBlocks;
    std::size_t blockSize;
    std::size_t numBlocks;
    std::size_t alignment;
    std::size_t alignedBlockSize;
    std::size_t totalMemorySize;
};

PoolAllocator::PoolAllocator(std::size_t blockSize, std::size_t numBlocks, std::size_t alignment)
    : m_impl(std::make_unique<Impl>(blockSize, numBlocks, alignment))
{
}

PoolAllocator::~PoolAllocator() = default;

PoolAllocator::PoolAllocator(PoolAllocator&&) noexcept = default;
PoolAllocator& PoolAllocator::operator=(PoolAllocator&&) noexcept = default;

void* PoolAllocator::allocate()
{
    if (m_impl->freeBlocks.empty()) {
        return nullptr;
    }

    void* block = m_impl->freeBlocks.top();
    m_impl->freeBlocks.pop();
    return block;
}

void PoolAllocator::deallocate(void* ptr)
{
    if (ptr && owns(ptr)) {
        m_impl->freeBlocks.push(ptr);
    }
}

std::size_t PoolAllocator::getBlockSize() const
{
    return m_impl->blockSize;
}

std::size_t PoolAllocator::getTotalBlocks() const
{
    return m_impl->numBlocks;
}

std::size_t PoolAllocator::getAvailableBlocks() const
{
    return m_impl->freeBlocks.size();
}

std::size_t PoolAllocator::getAlignment() const
{
    return m_impl->alignment;
}

std::size_t PoolAllocator::getTotalMemorySize() const
{
    return m_impl->totalMemorySize;
}

bool PoolAllocator::owns(const void* ptr) const
{
    if (!m_impl->memory || !ptr) {
        return false;
    }

    const char* p = static_cast<const char*>(ptr);
    const char* start = m_impl->memory;
    const char* end = start + m_impl->alignedBlockSize * m_impl->numBlocks;

    return p >= start && p < end;
}

void PoolAllocator::reset()
{
    while (!m_impl->freeBlocks.empty()) {
        m_impl->freeBlocks.pop();
    }

    if (m_impl->memory) {
        for (std::size_t i = 0; i < m_impl->numBlocks; ++i) {
            m_impl->freeBlocks.push(m_impl->memory + i * m_impl->alignedBlockSize);
        }
    }
}

} // namespace nap
