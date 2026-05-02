#include "managers/TFIDFRanker.h"
#include "utils/Tokenizer.h"
#include <cmath>

using namespace std;

TFIDFRanker::TFIDFRanker(const InvertedIndex& index) : index_(index) {}

double TFIDFRanker::score(const Gig& gig,
    const DataList<string>& queryTokens,
    int totalGigsInCorpus) const {
  
    if (queryTokens.size() == 0) return 0.0;
    if (totalGigsInCorpus <= 0) return 0.0;

   
    string corpus = gig.getTitle() + " " + gig.getDescription();
    DataList<string> gigTokens = Tokenizer::tokenize(corpus);

   
    if (gigTokens.size() == 0) return 0.0;

    double total = 0.0;

    for (int i = 0; i < queryTokens.size(); ++i) {
        const string& term = queryTokens.get(i);

        
        int tf = countOccurrences(gigTokens, term);
        if (tf == 0) continue;

        
        DataList<int> postingList = index_.findGigsContaining(term);
        int df = postingList.size();

       
        if (df == 0) continue;

        
        double idf = log(static_cast<double>(totalGigsInCorpus) /
            static_cast<double>(df));

        total += static_cast<double>(tf) * idf;
    }

   
    return total < 0.0 ? 0.0 : total;
}

int TFIDFRanker::countOccurrences(const DataList<string>& tokens,
    const string& token) {
    
    int count = 0;
    int n = tokens.size();
    for (int i = 0; i < n; ++i) {
        if (tokens.get(i) == token) ++count;
    }
    return count;
}