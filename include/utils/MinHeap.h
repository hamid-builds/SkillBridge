#ifndef SKILLBRIDGE_MIN_HEAP_H
#define SKILLBRIDGE_MIN_HEAP_H

#include <stdexcept>
#include <functional>
#include <utility>






template <typename T, typename Compare = std::less<T>>
class MinHeap {
private:
    T* data_;
    int     count_;
    int     capacity_;
    Compare cmp_;

  
    void grow() {
        int newCap = (capacity_ == 0) ? 8 : capacity_ * 2;
        T* newData = new T[newCap];
        for (int i = 0; i < count_; ++i) {
            newData[i] = data_[i];
        }
        delete[] data_;
        data_ = newData;
        capacity_ = newCap;
    }

    
    void siftUp(int idx) {
        while (idx > 0) {
            int parent = (idx - 1) / 2;
            if (cmp_(data_[idx], data_[parent])) {
                std::swap(data_[idx], data_[parent]);
                idx = parent;
            }
            else {
                break;
            }
        }
    }

   
    void siftDown(int idx) {
        while (true) {
            int left = 2 * idx + 1;
            int right = 2 * idx + 2;
            int best = idx;

            if (left < count_ && cmp_(data_[left], data_[best])) {
                best = left;
            }
            if (right < count_ && cmp_(data_[right], data_[best])) {
                best = right;
            }
            if (best == idx) break;

            std::swap(data_[idx], data_[best]);
            idx = best;
        }
    }

public:
  
    MinHeap() : data_(nullptr), count_(0), capacity_(0), cmp_(Compare()) {}

    
    explicit MinHeap(Compare cmp)
        : data_(nullptr), count_(0), capacity_(0), cmp_(cmp) {
    }

   
    ~MinHeap() {
        delete[] data_;
    }

   
    MinHeap(const MinHeap& other)
        : data_(nullptr), count_(other.count_), capacity_(other.capacity_),
        cmp_(other.cmp_) {
        if (capacity_ > 0) {
            data_ = new T[capacity_];
            for (int i = 0; i < count_; ++i) {
                data_[i] = other.data_[i];
            }
        }
    }

    MinHeap& operator=(const MinHeap& other) {
        if (this == &other) return *this;
        delete[] data_;
        count_ = other.count_;
        capacity_ = other.capacity_;
        cmp_ = other.cmp_;
        data_ = nullptr;
        if (capacity_ > 0) {
            data_ = new T[capacity_];
            for (int i = 0; i < count_; ++i) {
                data_[i] = other.data_[i];
            }
        }
        return *this;
    }

    
    void push(const T& value) {
        if (count_ == capacity_) grow();
        data_[count_] = value;
        ++count_;
        siftUp(count_ - 1);
    }

    const T& top() const {
        if (count_ == 0) {
            throw std::runtime_error("MinHeap::top called on empty heap");
        }
        return data_[0];
    }

    T pop() {
        if (count_ == 0) {
            throw std::runtime_error("MinHeap::pop called on empty heap");
        }
        T result = data_[0];
        --count_;
        if (count_ > 0) {
            data_[0] = data_[count_];
            siftDown(0);
        }
        return result;
    }

    int  size()    const { return count_; }
    bool isEmpty() const { return count_ == 0; }

    void clear() { count_ = 0; }
};

#endif