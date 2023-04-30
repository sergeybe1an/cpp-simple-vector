#pragma once
#include "array_ptr.h"
#include <stdexcept>
#include <algorithm>
#include <initializer_list>


class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity_to_reserve):
            capacity_to_reserve_(capacity_to_reserve) {}

    size_t GetCapacityToReserve() {
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
            :items_(size)
    {
        size_ = size;
        capacity_ = size;
        std::fill(begin(), end(), Type());
    }

    SimpleVector(size_t size, const Type& value)
            :items_(size)
    {
        size_ = size;
        capacity_ = size;
        std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init)
            :items_(init.size())
    {
        size_ = init.size();
        capacity_ = init.size();
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector(const SimpleVector& other)
            :items_(other.GetSize())
    {
        size_ = other.GetSize();
        capacity_ = other.GetCapacity();
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector(SimpleVector&& other)
            :items_(other.GetSize())
    {
        size_ = std::move(other.size_);
        capacity_ = std::move(other.capacity_);
        items_.swap(other.items_);
        other.Clear();
    }

    SimpleVector(ReserveProxyObj proxyObj) {
        Reserve(proxyObj.GetCapacityToReserve());
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

    SimpleVector& operator=(const SimpleVector& rhs) noexcept {
        if (this != &rhs) {
            SimpleVector temp(rhs);
            swap(temp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs) {
            swap(std::move(rhs));
            rhs.Clear();
        }
        return *this;
    }

    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    Type& At(size_t index) {
        if (index >= GetSize()) {
            throw std::out_of_range("Error: no index");
        }
        return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= GetSize()) {
            throw std::out_of_range("Error: no index");
        }
        return items_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size < size_ && new_size < capacity_) {
            size_ = new_size;
        } else if (new_size > size_ || new_size > capacity_) {
            ArrayPtr<Type> temp(new_size);
            std::copy(items_.Get(), items_.Get() + size_, temp.Get());

            items_.swap(temp);
            size_ = new_size;
            capacity_ = new_size;
        } else if (new_size > size_ || new_size < capacity_) {
            ArrayPtr<Type> temp(new_size);
            std::generate(items_.Get(), items_.Get() + size_, [] () { return Type(); });
            std::copy(items_.Get(), items_.Get() + size_, temp.Get());

            items_.swap(temp);
            size_ = new_size;
        }
    }

    void PushBack(const Type& value) noexcept {
        if (size_ == capacity_) {
            int new_capacity = capacity_ == 0 ? 1 : 2 * capacity_;
            ArrayPtr<Type> temp(new_capacity);
            std::copy(items_.Get(), items_.Get() + size_, temp.Get());

            items_.swap(temp);
            items_[size_] = value;
            ++size_;
            capacity_ = new_capacity;
        } else {
            ++size_;
            items_[size_ - 1] = value;
        }
    }

    void PushBack(Type&& value) noexcept {
        if (size_ == capacity_) {
            int new_capacity = capacity_ == 0 ? 1 : 2 * capacity_;
            ArrayPtr<Type> temp(new_capacity);
            std::move(items_.Get(), items_.Get() + size_, temp.Get());

            items_.swap(temp);
            items_[size_] = std::move(value);
            ++size_;
            capacity_ = new_capacity;
        } else {
            ++size_;
            items_[size_ - 1] = std::move(value);;
        }
    }

    void PopBack() noexcept {
        if (IsEmpty()) return;
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        auto index = std::distance(cbegin(), pos);
        auto it = begin() + index;
        std::move((it + 1), end(), it);
        --size_;
        return Iterator(pos);
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(other.size_, size_);
        std::swap(other.capacity_, capacity_);
    }

    void swap(SimpleVector&& other) noexcept {
        items_.swap(other.items_);
        std::swap(other.size_, size_);
        std::swap(other.capacity_, capacity_);
        other.Clear();
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t index = pos - begin();
        if (size_ < capacity_) {
            if (pos == end()) {
                items_[size_] = value;
            } else {
                std::copy_backward(items_.Get() + index, items_.Get() + size_, items_.Get() + size_ + 1);
                items_[index] = value;
            }
            ++size_;
        } else {
            size_t new_capacity = capacity_ == 0 ? 1 : 2 * capacity_;
            ArrayPtr<Type> new_data (new_capacity);

            std::copy(items_.Get(), items_.Get() + index, new_data.Get());
            new_data[index] = value;
            std::copy(items_.Get() + index, items_.Get() + size_, new_data.Get() + index + 1);

            items_.swap(new_data);
            capacity_ = new_capacity;
            ++size_;
        }
        return begin() + index;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        size_t index = pos - begin();
        if (size_ < capacity_) {
            if (pos == end()) {
                items_[size_] = std::move(value);
            } else {
                std::move_backward(items_.Get() + index, items_.Get() + size_, items_.Get() + size_ + 1);
                items_[index] = std::move(value);
            }
            ++size_;
        } else {
            size_t new_capacity = capacity_ == 0 ? 1 : 2 * capacity_;
            ArrayPtr<Type> new_data (new_capacity);

            std::move(items_.Get(), items_.Get() + index, new_data.Get());
            new_data[index] = std::move(value);
            std::move(items_.Get() + index, items_.Get() + size_, new_data.Get() + index + 1);

            items_.swap(new_data);
            capacity_ = new_capacity;
            ++size_;
        }
        return begin() + index;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }

        ArrayPtr<Type> tmp_items(new_capacity);
        for (std::size_t i{}; i < size_; ++i){
            tmp_items[i] = items_[i];
        }
        items_.swap(tmp_items);
        capacity_ = new_capacity;
    }


    Iterator begin() noexcept {
        return &items_[0];
    }

    Iterator end() noexcept {
        return &items_[size_];
    }


    ConstIterator begin() const noexcept {
        return &items_[0];
    }


    ConstIterator end() const noexcept {
        return &items_[size_];
    }


    ConstIterator cbegin() const noexcept {
        return &items_[0];
    }


    ConstIterator cend() const noexcept {
        return &items_[size_];
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
void swap(SimpleVector<Type>& lhs, SimpleVector<Type>& rhs) noexcept {
lhs.swap(rhs);
}

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {

    return (lhs.GetSize() == rhs.GetSize()) && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {

    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {

    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {

    return (lhs == rhs) || (lhs < rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {

    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {

    return (lhs == rhs) || (rhs < lhs);
}