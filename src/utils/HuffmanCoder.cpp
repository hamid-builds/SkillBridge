#include "utils/HuffmanCoder.h"
#include "utils/MinHeap.h"
#include <stdexcept>
#include <cstring>

using std::string;
using std::vector;

void HuffmanCoder::writeBit(vector<uint8_t>& buf, size_t& bitPos, bool one) {
    size_t byteIdx = bitPos / 8;
    size_t bitIdx = 7 - (bitPos % 8);
    if (byteIdx >= buf.size()) {
        buf.push_back(0);
    }
    if (one) {
        buf[byteIdx] |= static_cast<uint8_t>(1u << bitIdx);
    }
    ++bitPos;
}

bool HuffmanCoder::readBit(const vector<uint8_t>& buf, size_t bitPos) {
    size_t byteIdx = bitPos / 8;
    size_t bitIdx = 7 - (bitPos % 8);
    return (buf[byteIdx] >> bitIdx) & 1u;
}

void HuffmanCoder::countFrequencies(const string& input, int freq[256]) {
    for (int i = 0; i < 256; ++i) freq[i] = 0;
    for (char c : input) {
        ++freq[static_cast<uint8_t>(c)];
    }
}

HuffmanCoder::Node* HuffmanCoder::buildTree(const int freq[256]) {
    MinHeap<Node*, NodePtrCompare> heap;
    for (int b = 0; b < 256; ++b) {
        if (freq[b] > 0) {
            heap.push(new Node(static_cast<uint8_t>(b), freq[b]));
        }
    }
    if (heap.size() == 0) {
        return nullptr;
    }

    if (heap.size() == 1) {
        Node* only = heap.pop();
        Node* clone = new Node(only->byte, only->freq);
        Node* root = new Node(only, clone);
        return root;
    }
    while (heap.size() > 1) {
        Node* a = heap.pop();
        Node* b = heap.pop();
        heap.push(new Node(a, b));
    }
    return heap.pop();
}
void HuffmanCoder::buildCodesHelper(Node* node, const string& path,
    string codes[256]) {
    if (node == nullptr) return;
    if (node->isLeaf()) {
        codes[node->byte] = path.empty() ? string("0") : path;
        return;
    }
    buildCodesHelper(node->left, path + "0", codes);
    buildCodesHelper(node->right, path + "1", codes);
}

void HuffmanCoder::buildCodes(Node* root, string codes[256]) {
    for (int i = 0; i < 256; ++i) codes[i].clear();
    buildCodesHelper(root, string(), codes);
}

void HuffmanCoder::serializeTreeHelper(Node* node,
    vector<uint8_t>& bits,
    size_t& bitPos) {
    if (node == nullptr) return;
    if (node->isLeaf()) {
        writeBit(bits, bitPos, false);
        uint8_t b = node->byte;
        for (int i = 7; i >= 0; --i) {
            writeBit(bits, bitPos, ((b >> i) & 1u) != 0);
        }
        return;
    }
    writeBit(bits, bitPos, true);
    serializeTreeHelper(node->left, bits, bitPos);
    serializeTreeHelper(node->right, bits, bitPos);
}

void HuffmanCoder::serializeTree(Node* root,
    vector<uint8_t>& bits,
    size_t& outBitCount) {
    bits.clear();
    outBitCount = 0;
    if (root == nullptr) return;
    serializeTreeHelper(root, bits, outBitCount);
}

HuffmanCoder::Node* HuffmanCoder::deserializeTree(const vector<uint8_t>& bits,
    size_t totalBits,
    size_t& bitPos) {
    if (bitPos >= totalBits) {
        throw std::runtime_error("HuffmanCoder: truncated tree blob");
    }
    bool flag = readBit(bits, bitPos++);
    if (!flag) {
        // Leaf: read 8 more bits as the byte.
        if (bitPos + 8 > totalBits) {
            throw std::runtime_error("HuffmanCoder: truncated leaf byte");
        }
        uint8_t b = 0;
        for (int i = 7; i >= 0; --i) {
            if (readBit(bits, bitPos++)) {
                b |= static_cast<uint8_t>(1u << i);
            }
        }
        return new Node(b, 0);
    }
    Node* left = deserializeTree(bits, totalBits, bitPos);
    Node* right = deserializeTree(bits, totalBits, bitPos);
    return new Node(left, right);
}

void HuffmanCoder::deleteTree(Node* node) {
    if (node == nullptr) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

EncodedMessage HuffmanCoder::encode(const string& input) {
    EncodedMessage out;

    if (input.empty()) {
        return out;
    }
    int freq[256];
    countFrequencies(input, freq);
    Node* root = buildTree(freq);
    string codes[256];
    buildCodes(root, codes);
    serializeTree(root, out.serializedTree, out.treeBitCount);
    out.bits.clear();
    out.bitCount = 0;
    for (char c : input) {
        const string& code = codes[static_cast<uint8_t>(c)];
        for (char ch : code) {
            writeBit(out.bits, out.bitCount, ch == '1');
        }
    }
    deleteTree(root);
    return out;
}

string HuffmanCoder::decode(const EncodedMessage& message) {
    if (message.bitCount == 0 && message.treeBitCount == 0) {
        return string();
    }
    if (message.bitCount > message.bits.size() * 8) {
        throw std::runtime_error("HuffmanCoder: bit count exceeds buffer");
    }
    if (message.treeBitCount > message.serializedTree.size() * 8) {
        throw std::runtime_error("HuffmanCoder: tree bit count exceeds buffer");
    }
    size_t treeCursor = 0;
    Node* root = deserializeTree(message.serializedTree,
        message.treeBitCount, treeCursor);
    string out;

    Node* node = root;
    for (size_t i = 0; i < message.bitCount; ++i) {
        bool b = readBit(message.bits, i);
        node = b ? node->right : node->left;
        if (node == nullptr) {
            deleteTree(root);
            throw std::runtime_error("HuffmanCoder: invalid bit path");
        }
        if (node->isLeaf()) {
            out.push_back(static_cast<char>(node->byte));
            node = root;
        }
    }
    if (node != root) {
        deleteTree(root);
        throw std::runtime_error("HuffmanCoder: incomplete final code");
    }
    deleteTree(root);
    return out;
}