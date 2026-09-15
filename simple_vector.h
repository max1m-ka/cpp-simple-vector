#pragma once

#include "array_ptr.h"

#include <initializer_list>
#include <stdexcept>
#include <iterator>

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity_to_reserve)
    : capacity_to_reserve_(capacity_to_reserve) {
    }

    size_t GetCapacity() {
        return capacity_to_reserve_;
    }
private:
    size_t capacity_to_reserve_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size)
        : data_(size > 0 ? new Type[size]() : nullptr)
        , size_(size)
        , capacity_(size) {
    }

    SimpleVector(size_t size, const Type& value)
        : data_(size > 0 ? new Type[size] : nullptr)
        , size_(size)
        , capacity_(size) {

        for (size_t i = 0; i < size; ++i) {
            data_[i] = value;
        }
    }

    SimpleVector(std::initializer_list<Type> init)
        : data_(init.size() > 0 ? new Type[init.size()] : nullptr)
        , size_(init.size())
        , capacity_(init.size()) {

        size_t i = 0;
        for (const auto& value : init) {
            data_[i] = value;
            ++i;
        }
    }

    SimpleVector(const SimpleVector& other) 
    : data_(other.size_ > 0 ? new Type[other.size_] : nullptr)
    , size_(other.size_)
    , capacity_(other.size_) {
        for (size_t i = 0; i < other.size_; ++i) {
            data_[i] = other.data_[i];
        }
    }

    SimpleVector(SimpleVector&& other)
    : data_(std::move(other.data_))
    , size_(other.size_)
    , capacity_(other.capacity_) {
        other.size_ = 0;
        other.capacity_ = 0;
    }

    SimpleVector(ReserveProxyObj proxy) 
    : data_(proxy.GetCapacity() > 0 ? new Type[proxy.GetCapacity()] : nullptr)
    , size_(0)
    , capacity_(proxy.GetCapacity()){
    }

    ~SimpleVector() = default;

    SimpleVector& operator=(const SimpleVector& other) {
        if (this == &other) {
            return *this;
        }

        SimpleVector temp(other);
        swap(temp);

        return *this;
    }

    SimpleVector& operator=(SimpleVector&& other) noexcept{
        if (this == &other) {
            return *this;
        }
            data_ = std::move(other.data_);
            size_ = other.size_;
            capacity_ = other.capacity_;

            other.size_ = 0;
            other.capacity_ = 0;
        
        return *this;
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        return data_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return data_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }

        return data_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }

        return data_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
            return;
        }

        if (new_size <= capacity_) {
            for (size_t i = size_; i < new_size; ++i) {
                data_[i] = Type{};
            }

            size_ = new_size;
            return;
        }

        ArrayPtr<Type> new_data(new Type[new_size]());

        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data_[i]);
        }

        data_ = std::move(new_data);
        size_ = new_size;
        capacity_ = new_size;
    }

    void PushBack(const Type& item) {
        if (size_ < capacity_) {
            data_[size_] = item;
            ++size_;
            return;
        }

        const std::size_t new_capacity =
            capacity_ == 0 ? 1 : capacity_ * 2;

        ArrayPtr<Type> new_data(new Type[new_capacity]());

        for (std::size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data_[i]);
        }

        new_data[size_] = item;

        data_ = std::move(new_data);
        capacity_ = new_capacity;
        ++size_;
    }

    void PushBack(Type&& item) {
        if (size_ < capacity_) {
            data_[size_] = std::move(item);
            ++size_;
            return;
        }

        const std::size_t new_capacity =
            capacity_ == 0 ? 1 : capacity_ * 2;

        ArrayPtr<Type> new_data(new Type[new_capacity]());

        for (std::size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data_[i]);
        }

        new_data[size_] = std::move(item);

        data_ = std::move(new_data);

        capacity_ = new_capacity;
        ++size_;
    }

    void PopBack() noexcept{
        if (size_ == 0) 
            return;

        --size_;
    }

    Iterator Erase(ConstIterator pos) {
        const size_t index = pos - cbegin(); 

        for (size_t i = index; i + 1 < size_; ++i) { 
            data_[i] = std::move(data_[i + 1]); 
        } 

        --size_; 

        return data_.Get() + index;
    }

    void swap(SimpleVector& other) noexcept {
        data_.swap(other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    Iterator Insert(ConstIterator pos, Type& value) {
        return InsertImpl(pos, value);
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        return InsertImpl(pos, std::move(value));
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) 
            return;

        ArrayPtr<Type> new_data(new Type[new_capacity]());

        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data_[i]);
        }

        data_ = std::move(new_data);
        capacity_ = new_capacity;
    }

    Iterator begin() noexcept {
        return data_.Get();
    }

    Iterator end() noexcept {
        return data_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return data_.Get();
    }

    ConstIterator end() const noexcept {
        return data_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return data_.Get();
    }

    ConstIterator cend() const noexcept {
        return data_.Get() + size_;
    }

private:
    template <typename Value>
    Iterator InsertImpl(ConstIterator pos, Value&& value) {
        const std::size_t index = pos - cbegin();

        if (size_ == capacity_) {
            const std::size_t new_capacity =
                capacity_ == 0 ? 1 : capacity_ * 2;

            ArrayPtr<Type> new_data(new Type[new_capacity]());

            for (std::size_t i = 0; i < index; ++i) {
                new_data[i] = std::move(data_[i]);
            }

            new_data[index] = std::forward<Value>(value);

            for (std::size_t i = index; i < size_; ++i) {
                new_data[i + 1] = std::move(data_[i]);
            }

            data_ = std::move(new_data);

            capacity_ = new_capacity;
        } else {
            for (std::size_t i = size_; i > index; --i) {
                data_[i] = std::move(data_[i - 1]);
            }

            data_[index] = std::forward<Value>(value);
        }

        ++size_;

        return data_.Get() + index;
    }

private:
    ArrayPtr<Type> data_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template<typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize())
        return false;
    
    for (size_t i = 0; i < lhs.GetSize(); ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

template<typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    const size_t min_size = lhs.GetSize() < rhs.GetSize() ? lhs.GetSize() : rhs.GetSize();

    for (size_t i = 0; i < min_size; ++i) {
        if (lhs[i] < rhs[i]) {
            return true;
        }

        if (lhs[i] > rhs[i]) {
            return false;
        }
    }
    return lhs.GetSize() < rhs.GetSize();
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
} 
