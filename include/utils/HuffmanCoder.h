#ifndef SKILLBRIDGE_HUFFMAN_CODER_H
#define SKILLBRIDGE_HUFFMAN_CODER_H
#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>

struct EncodedMessage {
    std::vector<uint8_t> bits;
    size_t bitCount;
    std::vector<uint8_t> serializedTree;
    size_t treeBitCount;
    EncodedMessage() : bitCount(0), treeBitCount(0) {}
};

class HuffmanCoder {
public:
    EncodedMessage encode(const std::string& input);
    std::string decode(const EncodedMessage& message);

private:
    struct Node {
        uint8_t byte;
        int freq;
        Node* left;
        Node* right;
        Node(uint8_t b, int f) : byte(b), freq(f), left(nullptr), right(nullptr) {}
        Node(Node* l, Node* r) : byte(0), freq((l ? l->freq : 0) + (r ? r->freq : 0)), left(l), right(r) {}
        bool isLeaf() const {
            return left == nullptr && right == nullptr;
        }
    };

    struct NodePtrCompare {
        bool operator()(Node* a, Node* b) const {
            if (a->freq != b->freq)
                return a->freq < b->freq;
            if (a->isLeaf() != b->isLeaf())
                return a->isLeaf();
            return a->byte < b->byte;
        }
    };

    void countFrequencies(const std::string& input, int freq[256]);
    Node* buildTree(const int freq[256]);
    void buildCodes(Node* root, std::string codes[256]);
    void buildCodesHelper(Node* node, const std::string& path, std::string codes[256]);
    void serializeTree(Node* root, std::vector<uint8_t>& bits, size_t& outBitCount);
    void serializeTreeHelper(Node* node, std::vector<uint8_t>& bits, size_t& bitPos);
    Node* deserializeTree(const std::vector<uint8_t>& bits, size_t totalBits, size_t& bitPos);
    void deleteTree(Node* node);
    void writeBit(std::vector<uint8_t>& buf, size_t& bitPos, bool one);
    bool readBit(const std::vector<uint8_t>& buf, size_t bitPos);
};

#endif#pragma once
