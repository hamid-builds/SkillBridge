#ifndef SKILLBRIDGE_IFUZZY_MATCHER_H
#define SKILLBRIDGE_IFUZZY_MATCHER_H

#include <string>


class IFuzzyMatcher {
public:
    virtual ~IFuzzyMatcher() = default;

    
    virtual int distance(const std::string& a, const std::string& b) const = 0;
};

#endif