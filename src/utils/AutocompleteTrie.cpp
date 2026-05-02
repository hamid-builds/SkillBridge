#include "utils/AutocompleteTrie.h"
#include "utils/Sorting.h"

using namespace std;

AutocompleteTrie::AutocompleteTrie() : root_(new Node()), size_(0) {}

AutocompleteTrie::~AutocompleteTrie() {
    deleteSubtree(root_);
    root_ = nullptr;
}

void AutocompleteTrie::deleteSubtree(Node* node) {
    if (node == nullptr) return;
    for (int i = 0; i < 26; ++i) {
        deleteSubtree(node->children[i]);
    }
    delete node;
}

bool AutocompleteTrie::isAllLowerAlpha(const string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (c < 'a' || c > 'z') return false;
    }
    return true;
}

void AutocompleteTrie::insert(const string& word) {
    if (!isAllLowerAlpha(word)) return;

    Node* curr = root_;
    for (char c : word) {
        int idx = c - 'a';
        if (curr->children[idx] == nullptr) {
            curr->children[idx] = new Node();
        }
        curr = curr->children[idx];
    }

    if (!curr->isEnd) {
        curr->isEnd = true;
        size_++;
    }
    curr->frequency++;
}

const AutocompleteTrie::Node* AutocompleteTrie::findNode(const string& s) const {
    const Node* curr = root_;
    for (char c : s) {
        if (c < 'a' || c > 'z') return nullptr;
        int idx = c - 'a';
        if (curr->children[idx] == nullptr) return nullptr;
        curr = curr->children[idx];
    }
    return curr;
}

bool AutocompleteTrie::contains(const string& word) const {
    const Node* node = findNode(word);
    return node != nullptr && node->isEnd;
}

int AutocompleteTrie::frequencyOf(const string& word) const {
    const Node* node = findNode(word);
    if (node == nullptr || !node->isEnd) return 0;
    return node->frequency;
}

void AutocompleteTrie::collectAll(const Node* node,
    string& currentPath,
    DataList<WordWithFreq>& out) {
    if (node == nullptr) return;

    if (node->isEnd) {
        WordWithFreq w;
        w.word = currentPath;
        w.frequency = node->frequency;
        out.add(w);
    }

    for (int i = 0; i < 26; ++i) {
        if (node->children[i] != nullptr) {
            currentPath.push_back(static_cast<char>('a' + i));
            collectAll(node->children[i], currentPath, out);
            currentPath.pop_back();
        }
    }
}

void AutocompleteTrie::collectWordsOnly(const Node* node,
    string& currentPath,
    DataList<string>& out) {
    if (node == nullptr) return;

    if (node->isEnd) {
        out.add(currentPath);
    }

    for (int i = 0; i < 26; ++i) {
        if (node->children[i] != nullptr) {
            currentPath.push_back(static_cast<char>('a' + i));
            collectWordsOnly(node->children[i], currentPath, out);
            currentPath.pop_back();
        }
    }
}

DataList<string> AutocompleteTrie::findByPrefix(const string& prefix,
    int maxResults) const {
    DataList<string> results;
    if (maxResults <= 0) return results;

    const Node* startNode = findNode(prefix);
    if (startNode == nullptr) return results;

    DataList<WordWithFreq> all;
    string currentPath = prefix;
    collectAll(startNode, currentPath, all);

    mergeSort(all, [](const WordWithFreq& a, const WordWithFreq& b) {
        return a.frequency > b.frequency;
        });

    int n = all.size();
    int take = (n < maxResults) ? n : maxResults;
    for (int i = 0; i < take; ++i) {
        results.add(all.get(i).word);
    }

    return results;
}

DataList<string> AutocompleteTrie::getAllWords() const {
    DataList<string> result;
    string currentPath;
    collectWordsOnly(root_, currentPath, result);
    return result;
}

int AutocompleteTrie::size() const {
    return size_;
}

void AutocompleteTrie::clear() {
    deleteSubtree(root_);
    root_ = new Node();
    size_ = 0;
}