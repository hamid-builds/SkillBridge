#include "utils/Tokenizer.h"

using namespace std;

namespace {
    const char* STOPWORDS[] = {
        "a", "an", "and", "are", "as", "at", "be", "by", "for", "from",
        "has", "have", "he", "in", "is", "it", "its", "of", "on", "or",
        "that", "the", "this", "to", "was", "were", "will", "with", "you",
        "your", "i", "we", "they", "but", "not"
    };
    const int STOPWORD_COUNT = sizeof(STOPWORDS) / sizeof(STOPWORDS[0]);
}

bool Tokenizer::isStopword(const string& w) {
    
    for (int i = 0; i < STOPWORD_COUNT; ++i) {
        if (w == STOPWORDS[i]) return true;
    }
    return false;
}

string Tokenizer::toLower(const string& s) {
    string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c >= 'A' && c <= 'Z') {
            out += static_cast<char>(c - 'A' + 'a');
        }
        else {
            out += c;
        }
    }
    return out;
}

DataList<string> Tokenizer::tokenize(const string& text) {
    DataList<string> tokens;
    string current;
    current.reserve(32);  
    for (size_t i = 0; i <= text.size(); ++i) {
       
        char c = (i < text.size()) ? text[i] : ' ';

        bool isAlphaNum =
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9');

        if (isAlphaNum) {
          
            if (c >= 'A' && c <= 'Z') {
                current += static_cast<char>(c - 'A' + 'a');
            }
            else {
                current += c;
            }
        }
        else {
           
            if (!current.empty()) {
                if (current.size() >= 2 && !isStopword(current)) {
                    tokens.add(current);
                }
                current.clear();
            }
        }
    }

    return tokens;
}