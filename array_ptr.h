#pragma once

#include <cstddef>
#include <utility>

using namespace std;

template <typename Type>
class ArrayPtr {
public:
    ArrayPtr() noexcept = default;

    explicit ArrayPtr(Type* ptr) noexcept 
    : ptr_(ptr) {
    }

    ArrayPtr(const ArrayPtr& ) = delete;
    ArrayPtr& operator=(const ArrayPtr&) = delete;

    ArrayPtr(ArrayPtr&& other) noexcept
    : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    ArrayPtr& operator=(ArrayPtr&& other) {
        if (this != &other) {
            delete[] ptr_;
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    ~ArrayPtr() {
        delete[] ptr_;
    }

    Type* Get() const {
        return ptr_;
    }

    Type& operator[](size_t index) {
        return ptr_[index];
    }
    const Type& operator[](size_t index) const noexcept {
        return ptr_[index];
    }

    void swap(ArrayPtr& other) {
        std::swap(ptr_, other.ptr_);
    }
private:
    Type* ptr_ = nullptr;
};
