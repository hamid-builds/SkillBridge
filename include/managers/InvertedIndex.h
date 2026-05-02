#ifndef SKILLBRIDGE_INVERTED_INDEX_H
#define SKILLBRIDGE_INVERTED_INDEX_H

#include "core/Gig.h"
#include "utils/HashMap.h"
#include "utils/DataList.h"
#include <string>



    
    class InvertedIndex {
    public:
        InvertedIndex();

        void addGig(const Gig& gig);
        void removeGig(int gigID);

        
        DataList<int> findGigsContaining(const std::string& token) const;

       
        DataList<int> findGigsContainingAll(const DataList<std::string>& tokens) const;

        
        int getVocabularySize() const;

        bool containsToken(const std::string& token) const;
        void clear();

    private:
        HashMap<std::string, DataList<int>> index_;

       
        static void insertSorted(DataList<int>& postingList, int gigID);

        static DataList<int> intersectSorted(const DataList<int>& a,
            const DataList<int>& b);
    };



#endif 