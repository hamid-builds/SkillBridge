#ifndef SKILLBRIDGE_GIG_MANAGER_H
#define SKILLBRIDGE_GIG_MANAGER_H

#include "IGigRepository.h"
#include "IUserRepository.h"
#include "ITextRanker.h"
#include "IFuzzyMatcher.h"
#include "InvertedIndex.h"
#include "../utils/AutocompleteTrie.h"
#include "../utils/HashMap.h"
#include "../utils/DataList.h"
#include "../core/Gig.h"
#include "../core/GigCategory.h"
#include <string>





class GigManager {
public:
    
    GigManager(IGigRepository* gigRepo,
        IUserRepository* userRepo,
        ITextRanker* ranker,
        IFuzzyMatcher* matcher,
        InvertedIndex& sharedIndex);

    GigManager(const GigManager&) = delete;
    GigManager& operator=(const GigManager&) = delete;

   
    Gig createGig(int currentUserID,
        const std::string& title,
        const std::string& description,
        double price,
        GigCategory category);

    void updateGig(int currentUserID,
        int gigID,
        const std::string& title,
        const std::string& description,
        double price,
        GigCategory category);

    void deactivateGig(int currentUserID, int gigID);
    void deleteGig(int currentUserID, int gigID);

   
    Gig findGigByID(int gigID) const;
    DataList<Gig> findGigsByOwner(int ownerID) const;
    DataList<Gig> findAllActiveGigs() const;
    DataList<Gig> findAllGigs(int currentUserID) const;

   
    DataList<Gig> searchGigs(const std::string& rawQuery,
        int maxResults) const;

    DataList<std::string> autocompleteSuggestions(const std::string& prefix,
        int maxResults) const;

   
    void rebuildIndexes();

    int getVocabularySize() const;

private:
    IGigRepository* gigRepo_;
    IUserRepository* userRepo_;
    ITextRanker* ranker_;
    IFuzzyMatcher* matcher_;

    InvertedIndex& index_;       
    AutocompleteTrie  trie_;         
    HashMap<std::string, DataList<std::string>> soundexIndex_;  

   
    void indexGig(const Gig& gig);
    void unindexGig(int gigID);

   
    DataList<int> tryExactMatch(const DataList<std::string>& tokens) const;
    DataList<int> tryFuzzyMatch(const DataList<std::string>& tokens) const;
    DataList<int> trySoundexMatch(const DataList<std::string>& tokens) const;

    
    User* loadUser(int userID) const;
    static void requireAdmin(const User* user);
    static void requireOwnerOrAdmin(const User* user, const Gig& gig);
};

#endif