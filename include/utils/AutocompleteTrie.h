#ifndef SKILLBRIDGE_AUTOCOMPLETE_TRIE_H
#define SKILLBRIDGE_AUTOCOMPLETE_TRIE_H

#include "DataList.h"
#include <string>




class AutocompleteTrie {
public:
    AutocompleteTrie();
    ~AutocompleteTrie();

   
    AutocompleteTrie(const AutocompleteTrie&) = delete;
    AutocompleteTrie& operator=(const AutocompleteTrie&) = delete;

    
    void insert(const std::string& word);

    
    bool contains(const std::string& word) const;

   
    int frequencyOf(const std::string& word) const;

   
    DataList<std::string> findByPrefix(const std::string& prefix,
        int maxResults) const;

   
   
    DataList<std::string> getAllWords() const;

   
    int size() const;

   
    void clear();

private:
    struct Node {
        Node* children[26]; 
        bool  isEnd;        
        int   frequency;   

        Node() : isEnd(false), frequency(0) {
            for (int i = 0; i < 26; ++i) children[i] = nullptr;
        }
    };

    Node* root_;
    int   size_;

    
    static void deleteSubtree(Node* node);

   
    const Node* findNode(const std::string& s) const;

  
    struct WordWithFreq {
        std::string word;
        int frequency;
    };
    static void collectAll(const Node* node,
        std::string& currentPath,
        DataList<WordWithFreq>& out);

    
    static void collectWordsOnly(const Node* node,
        std::string& currentPath,
        DataList<std::string>& out);

   
    static bool isAllLowerAlpha(const std::string& s);
};

#endif