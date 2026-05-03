#pragma once
#include <string>


class ISentimentAnalyzer {
public:
    virtual ~ISentimentAnalyzer() = default;
   virtual double analyze(const std::string& text) const = 0;
    virtual std::string name() const = 0;
};