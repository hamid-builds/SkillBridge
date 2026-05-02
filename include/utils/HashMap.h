#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <string>
#include <cstdint>
#include <cstddef>
#include <stdexcept>

template <typename K>
struct HashMapHasher 
{
    static uint32_t hash(const K& key);
};

template <>
struct HashMapHasher<std::string> 
{
    static uint32_t hash(const std::string& key) 
    {
        uint32_t h = 5381;
        for (char c : key) 
        {
            h = ((h << 5) + h) + static_cast<uint8_t>(c);
        }
        return h;
    }
};

template <>
struct HashMapHasher<int> 
{
    static uint32_t hash(const int& key) 
    {
        uint32_t x = static_cast<uint32_t>(key);
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = (x >> 16) ^ x;
        return x;
    }
};

template <typename K, typename V>
class HashMap 
{
private:
    struct Node 
    {
        K key;
        V value;
        Node* next;
        Node(const K& k, const V& v, Node* n) : key(k), value(v), next(n) 
        {

        }
    };

    Node** buckets_;
    size_t bucketCount_;
    size_t size_;
    static constexpr double LOAD_FACTOR_THRESHOLD = 0.75;

    size_t bucketIndex(const K& key) const 
    {
        return HashMapHasher<K>::hash(key) % bucketCount_;
    }

    void resize() 
    {
        size_t oldCount = bucketCount_;
        Node** oldBuckets = buckets_;

        bucketCount_ = oldCount * 2;
        buckets_ = new Node * [bucketCount_];
        for (size_t i = 0; i < bucketCount_; i++) 
            buckets_[i] = nullptr;

        for (size_t i = 0; i < oldCount; i++) 
        {
            Node* node = oldBuckets[i];
            while (node) 
            {
                Node* next = node->next;
                size_t newIdx = HashMapHasher<K>::hash(node->key) % bucketCount_;
                node->next = buckets_[newIdx];
                buckets_[newIdx] = node;
                node = next;
            }
        }
        delete[] oldBuckets;
    }

    void freeAllNodes() 
    {
        for (size_t i = 0; i < bucketCount_; i++) 
        {
            Node* node = buckets_[i];
            while (node) 
            {
                Node* next = node->next;
                delete node;
                node = next;
            }
            buckets_[i] = nullptr;
        }
    }

public:
    explicit HashMap(size_t initialBuckets = 16) : buckets_(nullptr), bucketCount_(initialBuckets), size_(0) 
    {
        if (initialBuckets == 0) 
        {
            throw std::invalid_argument("HashMap: initialBuckets must be positive");
        }
        buckets_ = new Node * [bucketCount_];
        for (size_t i = 0; i < bucketCount_; i++) 
            buckets_[i] = nullptr;
    }

    ~HashMap() 
    {
        freeAllNodes();
        delete[] buckets_;
    }

    HashMap(const HashMap& other) : buckets_(nullptr), bucketCount_(other.bucketCount_), size_(0) 
    {
        buckets_ = new Node * [bucketCount_];
        for (size_t i = 0; i < bucketCount_; i++) 
            buckets_[i] = nullptr;

        for (size_t i = 0; i < other.bucketCount_; i++) 
        {
            Node* node = other.buckets_[i];
            while (node) 
            {
                put(node->key, node->value);
                node = node->next;
            }
        }
    }

    HashMap& operator=(const HashMap& other) 
    {
        if (this == &other) 
            return *this;
        freeAllNodes();
        delete[] buckets_;

        bucketCount_ = other.bucketCount_;
        size_ = 0;
        buckets_ = new Node * [bucketCount_];
        for (size_t i = 0; i < bucketCount_; i++) 
            buckets_[i] = nullptr;

        for (size_t i = 0; i < other.bucketCount_; i++) 
        {
            Node* node = other.buckets_[i];
            while (node) 
            {
                put(node->key, node->value);
                node = node->next;
            }
        }
        return *this;
    }
    void put(const K& key, const V& value) 
    {
        size_t idx = bucketIndex(key);

        Node* node = buckets_[idx];
        while (node) 
        {
            if (node->key == key) 
            {
                node->value = value;
                return;
            }
            node = node->next;
        }

        buckets_[idx] = new Node(key, value, buckets_[idx]);
        size_++;

        if (static_cast<double>(size_) / bucketCount_ > LOAD_FACTOR_THRESHOLD) 
        {
            resize();
        }
    }
    V* get(const K& key) 
    {
        size_t idx = bucketIndex(key);
        Node* node = buckets_[idx];
        while (node) 
        {
            if (node->key == key) 
                return &node->value;
            node = node->next;
        }
        return nullptr;
    }

    const V* get(const K& key) const 
    {
        size_t idx = bucketIndex(key);
        Node* node = buckets_[idx];
        while (node) 
        {
            if (node->key == key) 
                return &node->value;
            node = node->next;
        }
        return nullptr;
    }

    bool contains(const K& key) const 
    {
        return get(key) != nullptr;
    }

    bool remove(const K& key) 
    {
        size_t idx = bucketIndex(key);
        Node* node = buckets_[idx];
        Node* prev = nullptr;

        while (node) 
        {
            if (node->key == key) 
            {
                if (prev) prev->next = node->next;
                else buckets_[idx] = node->next;
                delete node;
                size_--;
                return true;
            }
            prev = node;
            node = node->next;
        }
        return false;
    }

    size_t size() const 
    {
        return size_; 
    }
    bool empty() const 
    {
        return size_ == 0; 
    }
    size_t bucketCount() const 
    {
        return bucketCount_; 
    }

    V& operator[](const K& key) 
    {
        size_t idx = bucketIndex(key);
        Node* node = buckets_[idx];
        while (node) 
        {
            if (node->key == key) return node->value;
            node = node->next;
        }

        buckets_[idx] = new Node(key, V(), buckets_[idx]);
        size_++;

        if (static_cast<double>(size_) / bucketCount_ > LOAD_FACTOR_THRESHOLD) 
        {
            resize();
            return *get(key);
        }
        return buckets_[idx]->value;
    }

    void clear() 
    {
        freeAllNodes();
        size_ = 0;
    }
   
          
    template <typename Fn>
    void forEach(Fn callback) {
        for (size_t i = 0; i < bucketCount_; i++) {
            Node* node = buckets_[i];
            while (node) {
                callback(node->key, node->value);
                node = node->next;
            }
        }
    }

    
    template <typename Fn>
    void forEach(Fn callback) const {
        for (size_t i = 0; i < bucketCount_; i++) {
            const Node* node = buckets_[i];
            while (node) {
                callback(node->key, node->value);
                node = node->next;
            }
        }
    }

};

#endif