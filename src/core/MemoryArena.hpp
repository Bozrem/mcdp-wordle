#pragma once

#include <cstdint>
#include <memory> // For unique_ptr stuff
#include <stdexcept>
#include <utility> // For std::forward
#include <mutex>

#include "RelPtr.hpp"

class MemoryArena {
    std::unique_ptr<std::byte[]> base_ptr;
    size_t capacity;
    size_t offset;
    std::mutex lock;

public:
    MemoryArena(uint64_t mb) : offset(0) {
        unsigned long long bytes = mb * 1024L * 1024L;
        base_ptr = std::make_unique<std::byte[]>(bytes);
        capacity = bytes;
    }

    // No destructor. Anything allocated needs to either be a POD or get destroyed before this

    template <typename T, typename... Args>
    RelPtr<T> create_object(Args&&... args) {
        uintptr_t aligned_addr;

        { // Critical section with the offset
            std::lock_guard<std::mutex> guard(lock);

            // Get current aligned address
            uintptr_t curr_addr = reinterpret_cast<uintptr_t>(base_ptr.get()) + offset;
            aligned_addr = (curr_addr + alignof(T) - 1) & ~(alignof(T) - 1);

            size_t needed = aligned_addr - curr_addr + sizeof(T);

            if (offset + needed > capacity)
                throw std::runtime_error("MemoryArena Out of Memory");

            offset += needed;
        }

        T* t_ptr = new (reinterpret_cast<T*>(aligned_addr)) T(std::forward<Args>(args)...);

        RelPtr<T> rel_ptr;
        rel_ptr = t_ptr;
        return rel_ptr;
    }

    template <typename T>
    RelPtr<T> malloc_raw(size_t count) {
        uintptr_t aligned_addr;

        { // Critical section with the offset
            std::lock_guard<std::mutex> guard(lock);

            uintptr_t curr_addr = reinterpret_cast<uintptr_t>(base_ptr.get()) + offset;
            aligned_addr = (curr_addr + alignof(T) - 1) & ~(alignof(T) - 1);

            size_t needed = aligned_addr - curr_addr + (count * sizeof(T));

            if (offset + needed > capacity)
                throw std::runtime_error("MemoryArena Out of Memory");

            offset += needed;
        }
        // Unlock, since new offset is set
        lock.unlock();

        RelPtr<T> rel_ptr;
        rel_ptr = reinterpret_cast<T*>(aligned_addr);
        return rel_ptr;
    }

    // Leaving checkpointing as serializiation within each object, run from the global
};
