#ifndef SKILLBRIDGE_GIGCATEGORY_H
#define SKILLBRIDGE_GIGCATEGORY_H

#include <string>
#include <stdexcept>



enum class GigCategory {
    DESIGN,
    WRITING,
    CODING,
    MARKETING,
    TUTORING,
    OTHER
};


inline std::string gigCategoryToString(GigCategory c) {
    switch (c) {
    case GigCategory::DESIGN:    return "DESIGN";
    case GigCategory::WRITING:   return "WRITING";
    case GigCategory::CODING:    return "CODING";
    case GigCategory::MARKETING: return "MARKETING";
    case GigCategory::TUTORING:  return "TUTORING";
    case GigCategory::OTHER:     return "OTHER";
    }
   
    return "OTHER";
}


inline GigCategory gigCategoryFromString(const std::string& s) {
   
    std::string upper;
    upper.reserve(s.size());
    for (char ch : s) {
        if (ch >= 'a' && ch <= 'z') upper += (char)(ch - 'a' + 'A');
        else upper += ch;
    }

    if (upper == "DESIGN")    return GigCategory::DESIGN;
    if (upper == "WRITING")   return GigCategory::WRITING;
    if (upper == "CODING")    return GigCategory::CODING;
    if (upper == "MARKETING") return GigCategory::MARKETING;
    if (upper == "TUTORING")  return GigCategory::TUTORING;
    if (upper == "OTHER")     return GigCategory::OTHER;

    throw std::invalid_argument("Unknown gig category: " + s);
}

#endif