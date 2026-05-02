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

   
    
    struct TechSub {
        const char* from; 
        const char* to;  
    };
    const TechSub TECH_SUBS[] = {
        { "c++",  "cpp"     },
        { "c#",   "csharp"  },
        { "f#",   "fsharp"  },
        { ".net", "dotnet"  },
        { "node.js", "nodejs" },
    };
    const int TECH_SUB_COUNT = sizeof(TECH_SUBS) / sizeof(TECH_SUBS[0]);

    
    string applyTechSubs(const string& text) {
     
        string lower;
        lower.reserve(text.size());
        for (char c : text) {
            if (c >= 'A' && c <= 'Z') {
                lower += static_cast<char>(c - 'A' + 'a');
            }
            else {
                lower += c;
            }
        }

       
        for (int s = 0; s < TECH_SUB_COUNT; ++s) {
            const string from = TECH_SUBS[s].from;
            const string to = TECH_SUBS[s].to;
            string out;
            out.reserve(lower.size());
            size_t i = 0;
            while (i < lower.size()) {
                if (i + from.size() <= lower.size() &&
                    lower.compare(i, from.size(), from) == 0) {
                    out += to;
                    i += from.size();
                }
                else {
                    out += lower[i];
                    ++i;
                }
            }
            lower = out;
        }
        return lower;
    }
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

   
    string preprocessed = applyTechSubs(text);

    string current;
    current.reserve(32);

    for (size_t i = 0; i <= preprocessed.size(); ++i) {
        char c = (i < preprocessed.size()) ? preprocessed[i] : ' ';
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