#pragma once
#include "ISentimentAnalyzer.h"
#include <string>


class BagOfWordsSentiment : public ISentimentAnalyzer 
{
public:
    BagOfWordsSentiment();
    ~BagOfWordsSentiment() override = default;

    BagOfWordsSentiment(const BagOfWordsSentiment&) = delete;
    BagOfWordsSentiment& operator=(const BagOfWordsSentiment&) = delete;

    double      analyze(const std::string& text) const override;
    std::string name()    const override;

private:
    static void tokenize(const std::string& text,
        std::string        tokens[],
        int& count);

    static std::string stripPunct(const std::string& token);

    bool isPositive(const std::string& token) const;
    bool isNegative(const std::string& token) const;

    static const int  MAX_DICT = 64;
    std::string posWords_[MAX_DICT];
    std::string negWords_[MAX_DICT];
    int         posCount_ = 0;
    int         negCount_ = 0;

    bool inArray(const std::string arr[], int count, const std::string& word) const;
};