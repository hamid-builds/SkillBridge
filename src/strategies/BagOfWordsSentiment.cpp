
#include "../../include/strategies/BagOfWordsSentiment.h"
#include <cctype>
#include <algorithm>

static const char* POS_WORDS[] = {
    "accurate",   "amazing",    "awesome",    "best",
    "brilliant",  "clear",      "commend",    "competent",
    "creative",   "dedicated",  "delight",    "dependable",
    "efficient",  "excellent",  "exceptional","expert",
    "fast",       "fantastic",  "flawless",   "friendly",
    "genius",     "great",      "helpful",    "honest",
    "impressed",  "innovative", "kind",       "knowledgeable",
    "love",       "outstanding","perfect",    "pleased",
    "polite",     "professional","prompt",    "quality",
    "recommend",  "reliable",   "responsive", "satisfied",
    "skilled",    "smooth",     "stellar",    "superb",
    "talented",   "terrific",   "thorough",   "timely",
    "trustworthy","wonderful"
};

static const char* NEG_WORDS[] = {
    "awful",      "bad",        "broken",     "careless",
    "confusing",  "corrupt",    "defective",  "delay",
    "disappoint", "dishonest",  "disrespect", "dreadful",
    "error",      "fail",       "fake",       "flawed",
    "fraud",      "ghost",      "horrible",   "ignore",
    "incomplete", "incompetent","incorrect",  "inefficient",
    "late",       "lazy",       "mediocre",   "mislead",
    "mistake",    "negligent",  "never",      "overcharge",
    "overpriced", "plagiarize", "poor",       "rude",
    "scam",       "slow",       "sloppy",     "terrible",
    "unacceptable","unclear",   "unhelpful",  "unprofessional",
    "unreliable", "useless",    "waste",      "worst",
    "wrong",      "zero"
};

static const int POS_SIZE = static_cast<int>(sizeof(POS_WORDS) / sizeof(POS_WORDS[0]));
static const int NEG_SIZE = static_cast<int>(sizeof(NEG_WORDS) / sizeof(NEG_WORDS[0]));

BagOfWordsSentiment::BagOfWordsSentiment() {
    posCount_ = (POS_SIZE < MAX_DICT) ? POS_SIZE : MAX_DICT;
    for (int i = 0; i < posCount_; ++i)
        posWords_[i] = POS_WORDS[i];

    negCount_ = (NEG_SIZE < MAX_DICT) ? NEG_SIZE : MAX_DICT;
    for (int i = 0; i < negCount_; ++i)
        negWords_[i] = NEG_WORDS[i];
}

std::string BagOfWordsSentiment::name() const {
    return "BagOfWordsSentiment";
}

double BagOfWordsSentiment::analyze(const std::string& text) const {
    const int MAX_TOKENS = 512;
    std::string tokens[MAX_TOKENS];
    int count = 0;
    tokenize(text, tokens, count);

    if (count == 0) return 0.0;

    int pos = 0, neg = 0;
    for (int i = 0; i < count; ++i) {
        if (isPositive(tokens[i])) ++pos;
        else if (isNegative(tokens[i])) ++neg;
    }

    double raw = static_cast<double>(pos - neg) / static_cast<double>(count);
    if (raw > 1.0) return  1.0;
    if (raw < -1.0) return -1.0;
    return raw;
}

void BagOfWordsSentiment::tokenize(const std::string& text,
    std::string        tokens[],
    int& count) {
    count = 0;
    std::string current;
    for (size_t i = 0; i <= text.size(); ++i) {
        char c = (i < text.size()) ? text[i] : ' ';
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!current.empty()) {
                for (char& ch : current)
                    ch = static_cast<char>(
                        std::tolower(static_cast<unsigned char>(ch)));
                std::string stripped = stripPunct(current);
                if (!stripped.empty() && count < 512)
                    tokens[count++] = stripped;
                current.clear();
            }
        }
        else {
            current += c;
        }
    }
}

std::string BagOfWordsSentiment::stripPunct(const std::string& token) {
    int start = 0;
    int end = static_cast<int>(token.size()) - 1;
    while (start <= end &&
        std::ispunct(static_cast<unsigned char>(token[start])))
        ++start;
    while (end >= start &&
        std::ispunct(static_cast<unsigned char>(token[end])))
        --end;
    if (start > end) return "";
    return token.substr(start, end - start + 1);
}

bool BagOfWordsSentiment::isPositive(const std::string& token) const {
    return inArray(posWords_, posCount_, token);
}

bool BagOfWordsSentiment::isNegative(const std::string& token) const {
    return inArray(negWords_, negCount_, token);
}

bool BagOfWordsSentiment::inArray(const std::string arr[], int count,
    const std::string& word) const {
    int lo = 0, hi = count - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (arr[mid] == word) return true;
        else if (arr[mid] < word) lo = mid + 1;
        else                       hi = mid - 1;
    }
    return false;
}