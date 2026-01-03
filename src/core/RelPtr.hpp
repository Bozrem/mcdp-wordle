#pragma once
#include <cstdint>
#include <cstddef>
#include <stdexcept>

template <typename T>
class RelPtr {
    int64_t offset;

public:
    RelPtr() : offset(0) {
        // All get initialized as 0
    }

    RelPtr& operator=(T* ptr) {
        offset = ptr ? reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(this) : 0; // Implicit ptr int to int64
        return *this;
    }

    bool operator==(RelPtr other) {
        return reinterpret_cast<uintptr_t>(this) + offset == reinterpret_cast<uintptr_t>(other) + other->offset;
    }

    // I know it would be cleaner to reuse these, but I'm not sure if that would create a functional overhead
    T* operator->() const {
        if (offset == 0) throw std::runtime_error("Null RelPtr Access");
        return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this)+offset);
    }

    // Normal derefernece
    T& operator*() const {
        if (offset == 0) throw std::runtime_error("Null RelPtr Access");
        return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this)+offset);
    }

    // Allows array indexing
    T& operator[](std::ptrdiff_t index) const {
        if (offset == 0) throw std::runtime_error("Null RelPtr Access");
        T* base = *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this)+offset+index);
        return base[index];
    }

    // Allows    if (rel_ptr)
    explicit operator bool() const {
        return offset != 0;
    }
};
