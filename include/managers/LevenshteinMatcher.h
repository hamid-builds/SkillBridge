#ifndef SKILLBRIDGE_LEVENSHTEIN_MATCHER_H
#define SKILLBRIDGE_LEVENSHTEIN_MATCHER_H

#include "IFuzzyMatcher.h"


class LevenshteinMatcher : public IFuzzyMatcher {
public:
    LevenshteinMatcher() = default;

    int distance(const std::string& a, const std::string& b) const override;
};

#endif