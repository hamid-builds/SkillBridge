#ifndef SKILLBRIDGE_TFIDF_RANKER_H
#define SKILLBRIDGE_TFIDF_RANKER_H

#include "ITextRanker.h"
#include "InvertedIndex.h"

class TFIDFRanker : public ITextRanker {
public:
    
    explicit TFIDFRanker(const InvertedIndex& index);

   
    double score(const Gig& gig,
        const DataList<std::string>& queryTokens,
        int totalGigsInCorpus) const override;

private:
    const InvertedIndex& index_;

    
    static int countOccurrences(const DataList<std::string>& tokens,
        const std::string& token);
};

#endif