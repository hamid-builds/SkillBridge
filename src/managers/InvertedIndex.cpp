#include "managers/InvertedIndex.h"
#include "utils/Tokenizer.h"

using namespace std;



    InvertedIndex::InvertedIndex() : index_() {}

    void InvertedIndex::addGig(const Gig& gig) {
        
        string corpus = gig.getTitle() + " " + gig.getDescription();
        DataList<string> tokens = Tokenizer::tokenize(corpus);

        int gigID = gig.getGigID();

        for (int i = 0; i < tokens.size(); ++i) {
            const string& token = tokens.get(i);

           
            DataList<int>* postingList = index_.get(token);

            if (postingList == nullptr) {
               
                index_.put(token, DataList<int>());
                postingList = index_.get(token);
            }

            insertSorted(*postingList, gigID);
        }
    }

    void InvertedIndex::removeGig(int gigID) {
        
        index_.forEach([gigID](const string& /*token*/, DataList<int>& postingList) {
            for (int j = 0; j < postingList.size(); ++j) {
                int v = postingList.get(j);
                if (v == gigID) {
                    postingList.removeAt(j);
                    return; 
                }
                if (v > gigID) {
                    
                    return;
                }
            }
            });
    }

    DataList<int> InvertedIndex::findGigsContaining(const string& token) const {
        const DataList<int>* postingList = index_.get(token);
        if (postingList == nullptr) {
            return DataList<int>();
        }
       
        return *postingList;
    }

    DataList<int> InvertedIndex::findGigsContainingAll(
        const DataList<string>& tokens) const {

        if (tokens.size() == 0) {
            return DataList<int>();
        }

        const DataList<int>* first = index_.get(tokens.get(0));
        if (first == nullptr) {
            return DataList<int>();
        }

        DataList<int> result = *first;

        for (int i = 1; i < tokens.size(); ++i) {
            const DataList<int>* next = index_.get(tokens.get(i));
            if (next == nullptr) {
                return DataList<int>();
            }
            result = intersectSorted(result, *next);
            if (result.size() == 0) {
                return result; 
            }
        }

        return result;
    }

    int InvertedIndex::getVocabularySize() const {
        
        int count = 0;
        index_.forEach([&count](const string& /*token*/, const DataList<int>& list) {
            if (list.size() > 0) {
                ++count;
            }
            });
        return count;
    }

    bool InvertedIndex::containsToken(const string& token) const {
        const DataList<int>* postingList = index_.get(token);
        if (postingList == nullptr) {
            return false;
        }
        
        return postingList->size() > 0;
    }

    void InvertedIndex::clear() {
        index_.clear();
    }

    void InvertedIndex::insertSorted(DataList<int>& postingList, int gigID) {
        int n = postingList.size();
        for (int i = 0; i < n; ++i) {
            if (postingList.get(i) == gigID) {
                return; 
            }
            if (postingList.get(i) > gigID) {
               
                postingList.add(gigID);
                
                int newIdx = postingList.size() - 1;
                while (newIdx > i) {
                    int tmp = postingList.get(newIdx);
                    postingList[newIdx] = postingList.get(newIdx - 1);
                    postingList[newIdx - 1] = tmp;
                    --newIdx;
                }
                return;
            }
        }

       
        postingList.add(gigID);
    }

    DataList<int> InvertedIndex::intersectSorted(const DataList<int>& a,
        const DataList<int>& b) {
      
        DataList<int> result;
        int i = 0, j = 0;
        int na = a.size(), nb = b.size();

        while (i < na && j < nb) {
            int av = a.get(i);
            int bv = b.get(j);

            if (av == bv) {
                result.add(av);
                ++i;
                ++j;
            }
            else if (av < bv) {
                ++i;
            }
            else {
                ++j;
            }
        }

        return result;
    }

 