#ifndef SKILLBRIDGE_DATALIST_H
#define SKILLBRIDGE_DATALIST_H

#include <stdexcept>
#include <string>

template <typename T>
class DataList {
private:
    T* data;
    int count;
    int capacity;

    void grow() 
    {
        int newCapacity = (capacity == 0) ? 4 : capacity * 2;
        T* newData = new T[newCapacity];
        for (int i = 0; i < count; i++) 
        {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
        capacity = newCapacity;
    }

public:
    DataList() : data(nullptr), count(0), capacity(0) 
    {

    }

    ~DataList() 
    {
        delete[] data;
    }

    DataList(const DataList& other) : data(nullptr), count(other.count), capacity(other.capacity)
    {
        if (capacity > 0) 
        {
            data = new T[capacity];
            for (int i = 0; i < count; i++) 
            {
                data[i] = other.data[i];
            }
        }
    }

    DataList& operator=(const DataList& other) 
    {
        if (this == &other) return *this;

        delete[] data;
        count = other.count;
        capacity = other.capacity;
        data = nullptr;
        if (capacity > 0) 
        {
            data = new T[capacity];
            for (int i = 0; i < count; i++) 
            {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    void add(const T& item) 
    {
        if (count == capacity) 
        {
            grow();
        }
        data[count++] = item;
    }

    void removeAt(int index) 
    {
        if (index < 0 || index >= count) 
        {
            throw std::out_of_range("DataList index out of range: " + std::to_string(index));
        }
        for (int i = index; i < count - 1; i++) 
        {
            data[i] = data[i + 1];
        }
        count--;
    }

    void clear() 
    {
        count = 0;
    }

    T& get(int index) 
    {
        if (index < 0 || index >= count) 
        {
            throw std::out_of_range("DataList index out of range: " + std::to_string(index));
        }
        return data[index];
    }

    const T& get(int index) const 
    {
        if (index < 0 || index >= count) 
        {
            throw std::out_of_range("DataList index out of range: " + std::to_string(index));
        }
        return data[index];
    }
    T& operator[](int index) 
    {
        return get(index); 
    }
    const T& operator[](int index) const 
    {
        return get(index); 
    }

    int size() const 
    {
        return count; 
    }
    bool isEmpty() const 
    {
        return count == 0; 
    }
};

#endif