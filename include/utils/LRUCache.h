#ifndef SKILLBRIDGE_LRUCACHE_H
#define SKILLBRIDGE_LRUCACHE_H

#include <cstddef>
#include "utils/HashMap.h"

template <typename K, typename V>
class LRUCache {
private:
    struct Node {
        K key;
        V value;
        Node* prev;
        Node* next;
        Node(const K& k, const V& v) : key(k), value(v), prev(nullptr), next(nullptr) {}
    };

    HashMap<K, Node*> index_;
    Node* head_;
    Node* tail_;
    size_t capacity_;
    size_t size_;

    void detach_(Node* node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        node->prev = nullptr;
        node->next = nullptr;
    }

    void insertFront_(Node* node) {
        node->next = head_->next;
        node->prev = head_;
        head_->next->prev = node;
        head_->next = node;
    }

    void moveToFront_(Node* node) {
        detach_(node);
        insertFront_(node);
    }

    void evictLRU_() {
        if (size_ == 0)
            return;
        Node* victim = tail_->prev;
        detach_(victim);
        index_.remove(victim->key);
        delete victim;
        --size_;
    }

public:
    explicit LRUCache(size_t capacity)
        : index_(),
        head_(new Node(K(), V())), tail_(new Node(K(), V())), capacity_(capacity == 0 ? 1 : capacity), size_(0){
        head_->next = tail_;
        tail_->prev = head_;
    }

    ~LRUCache() {
        Node* curr = head_->next;
        while (curr != tail_) {
            Node* nx = curr->next;
            delete curr;
            curr = nx;
        }
        delete head_;
        delete tail_;
    }

    LRUCache(const LRUCache&) = delete;
    LRUCache& operator=(const LRUCache&) = delete;
    LRUCache(LRUCache&&) = delete;
    LRUCache& operator=(LRUCache&&) = delete;

    void put(const K& key, const V& value) {
        Node** existing = index_.get(key);
        if (existing != nullptr) {
            (*existing)->value = value;
            moveToFront_(*existing);
            return;
        }
        Node* node = new Node(key, value);
        if (size_ >= capacity_) {
            evictLRU_();
        }
        insertFront_(node);
        index_.put(key, node);
        ++size_;
    }

    V* get(const K& key) {
        Node** slot = index_.get(key);
        if (slot == nullptr)
            return nullptr;
        moveToFront_(*slot);
        return &((*slot)->value);
    }

    bool contains(const K& key) const {
        return index_.get(key) != nullptr;
    }

    bool remove(const K& key) {
        Node** slot = index_.get(key);
        if (slot == nullptr)
            return false;
        Node* node = *slot;
        detach_(node);
        index_.remove(key);
        delete node;
        --size_;
        return true;
    }

    void clear() {
        Node* curr = head_->next;
        while (curr != tail_) {
            Node* nx = curr->next;
            delete curr;
            curr = nx;
        }
        head_->next = tail_;
        tail_->prev = head_;
        index_.clear();
        size_ = 0;
    }

    size_t size() const {
        return size_;
    }
    size_t capacity() const {
        return capacity_;
    }
    bool   empty() const {
        return size_ == 0;
    }
};

#endif