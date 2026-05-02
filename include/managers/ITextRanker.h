#ifndef SKILLBRIDGE_ITEXT_RANKER_H
#define SKILLBRIDGE_ITEXT_RANKER_H

#include "../core/Gig.h"
#include "../utils/DataList.h"
#include <string>


class ITextRanker {
public:
    virtual ~ITextRanker() = default;

   
    virtual double score(const Gig& gig,
        const DataList<std::string>& queryTokens,
        int totalGigsInCorpus) const = 0;
};

#endif