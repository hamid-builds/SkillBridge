#include "managers/GigManager.h"
#include "core/User.h"
#include "core/UserRole.h"
#include "core/Exceptions.h"
#include "utils/Tokenizer.h"
#include "utils/SoundexEncoder.h"
#include "utils/Sorting.h"

#include <stdexcept>

using namespace std;

static constexpr int FUZZY_THRESHOLD = 2;


GigManager::GigManager(IGigRepository* gigRepo,
    IUserRepository* userRepo,
    ITextRanker* ranker,
    IFuzzyMatcher* matcher,
    InvertedIndex& sharedIndex)
    : gigRepo_(gigRepo),
    userRepo_(userRepo),
    ranker_(ranker),
    matcher_(matcher),
    index_(sharedIndex),
    trie_(),
    soundexIndex_(64) {
    if (!gigRepo_ || !userRepo_ || !ranker_ || !matcher_) {
        throw invalid_argument(
            "GigManager: all dependencies must be non-null");
    }
}


User* GigManager::loadUser(int userID) const {
    User* u = userRepo_->findUserByID(userID);
    if (!u) {
        throw AuthenticationException(
            "user not found: id=" + to_string(userID));
    }
    return u;
}

void GigManager::requireAdmin(const User* user) {
    if (user->getRole() != UserRole::ADMIN) {
        throw UnauthorizedException("operation requires Admin role");
    }
}

void GigManager::requireOwnerOrAdmin(const User* user, const Gig& gig) {
    bool isOwner = (user->getUserID() == gig.getOwnerID());
    bool isAdmin = (user->getRole() == UserRole::ADMIN);
    if (!isOwner && !isAdmin) {
        throw UnauthorizedException(
            "operation requires gig ownership or Admin role");
    }
}


void GigManager::indexGig(const Gig& gig) {
    index_.addGig(gig);

    string corpus = gig.getTitle() + " " + gig.getDescription();
    DataList<string> tokens = Tokenizer::tokenize(corpus);

    for (int i = 0; i < tokens.size(); ++i) {
        const string& tok = tokens.get(i);
        trie_.insert(tok);

        string code = SoundexEncoder::encode(tok);
        DataList<string>* list = soundexIndex_.get(code);
        if (list == nullptr) {
            soundexIndex_.put(code, DataList<string>());
            list = soundexIndex_.get(code);
        }
        bool already = false;
        for (int j = 0; j < list->size(); ++j) {
            if (list->get(j) == tok) { already = true; break; }
        }
        if (!already) {
            list->add(tok);
        }
    }
}

void GigManager::unindexGig(int gigID) {
    index_.removeGig(gigID);
}


Gig GigManager::createGig(int currentUserID,
    const string& title,
    const string& description,
    double price,
    GigCategory category) {
    User* user = loadUser(currentUserID);
    UserRole role = user->getRole();
    delete user;

    if (role != UserRole::FREELANCER) {
        throw UnauthorizedException("only Freelancers can create gigs");
    }

    Gig g(currentUserID, title, description, price, category);

    gigRepo_->saveGig(g);
    indexGig(g);

    return g;
}

void GigManager::updateGig(int currentUserID,
    int gigID,
    const string& title,
    const string& description,
    double price,
    GigCategory category) {
    User* user = loadUser(currentUserID);
    Gig existing = gigRepo_->findGigByID(gigID);

    try {
        requireOwnerOrAdmin(user, existing);
    }
    catch (...) {
        delete user;
        throw;
    }
    delete user;

    existing.setTitle(title);
    existing.setDescription(description);
    existing.setPrice(price);
    existing.setCategory(category);

    gigRepo_->updateGig(existing);

    unindexGig(gigID);
    indexGig(existing);
}

void GigManager::deactivateGig(int currentUserID, int gigID) {
    User* user = loadUser(currentUserID);
    Gig existing = gigRepo_->findGigByID(gigID);

    try {
        requireOwnerOrAdmin(user, existing);
    }
    catch (...) {
        delete user;
        throw;
    }
    delete user;

    gigRepo_->deactivateGig(gigID);
    unindexGig(gigID);
}

void GigManager::deleteGig(int currentUserID, int gigID) {
    User* user = loadUser(currentUserID);

    try {
        requireAdmin(user);
    }
    catch (...) {
        delete user;
        throw;
    }
    delete user;

    Gig existing = gigRepo_->findGigByID(gigID);
    (void)existing;

    gigRepo_->deleteGig(gigID);
    unindexGig(gigID);
}


Gig GigManager::findGigByID(int gigID) const {
    return gigRepo_->findGigByID(gigID);
}

DataList<Gig> GigManager::findGigsByOwner(int ownerID) const {
    return gigRepo_->findGigsByOwner(ownerID);
}

DataList<Gig> GigManager::findAllActiveGigs() const {
    return gigRepo_->findAllActiveGigs();
}

DataList<Gig> GigManager::findAllGigs(int currentUserID) const {
    User* user = loadUser(currentUserID);
    try {
        requireAdmin(user);
    }
    catch (...) {
        delete user;
        throw;
    }
    delete user;
    return gigRepo_->findAllGigs();
}


DataList<int> GigManager::tryExactMatch(const DataList<string>& tokens) const {
    return index_.findGigsContainingAll(tokens);
}

DataList<int> GigManager::tryFuzzyMatch(const DataList<string>& tokens) const {
    DataList<string> vocabulary = trie_.getAllWords();
    if (vocabulary.size() == 0) return DataList<int>();

    DataList<string> substituted;

    for (int i = 0; i < tokens.size(); ++i) {
        const string& tok = tokens.get(i);

        if (trie_.contains(tok)) {
            substituted.add(tok);
            continue;
        }

        int bestDist = FUZZY_THRESHOLD + 1;
        string bestWord;
        for (int j = 0; j < vocabulary.size(); ++j) {
            const string& v = vocabulary.get(j);
            int d = matcher_->distance(tok, v);
            if (d < bestDist) {
                bestDist = d;
                bestWord = v;
                if (d == 0) break;
            }
        }

        if (bestDist > FUZZY_THRESHOLD) {
            return DataList<int>();
        }
        substituted.add(bestWord);
    }

    return index_.findGigsContainingAll(substituted);
}

DataList<int> GigManager::trySoundexMatch(
    const DataList<string>& tokens) const {
    DataList<string> substituted;

    for (int i = 0; i < tokens.size(); ++i) {
        const string& tok = tokens.get(i);

        if (trie_.contains(tok)) {
            substituted.add(tok);
            continue;
        }

        string code = SoundexEncoder::encode(tok);
        const DataList<string>* candidates = soundexIndex_.get(code);
        if (candidates == nullptr || candidates->size() == 0) {
            return DataList<int>();
        }
        substituted.add(candidates->get(0));
    }

    return index_.findGigsContainingAll(substituted);
}

DataList<Gig> GigManager::searchGigs(const string& rawQuery,
    int maxResults) const {
    DataList<Gig> empty;
    if (maxResults <= 0) return empty;

    DataList<string> queryTokens = Tokenizer::tokenize(rawQuery);
    if (queryTokens.size() == 0) return empty;

    DataList<int> matchedIDs = tryExactMatch(queryTokens);
    if (matchedIDs.size() == 0) {
        matchedIDs = tryFuzzyMatch(queryTokens);
    }
    if (matchedIDs.size() == 0) {
        matchedIDs = trySoundexMatch(queryTokens);
    }
    if (matchedIDs.size() == 0) return empty;

    DataList<Gig> activeGigs = gigRepo_->findAllActiveGigs();
    int N = activeGigs.size();

    struct Scored {
        Gig gig;
        double score;
    };
    DataList<Scored> scored;
    for (int i = 0; i < matchedIDs.size(); ++i) {
        try {
            Gig g = gigRepo_->findGigByID(matchedIDs.get(i));
            Scored s{ g, ranker_->score(g, queryTokens, N) };
            scored.add(s);
        }
        catch (const GigNotFoundException&) {
            continue;
        }
    }

    mergeSort(scored, [](const Scored& a, const Scored& b) {
        return a.score > b.score;
        });

    DataList<Gig> results;
    int take = (scored.size() < maxResults) ? scored.size() : maxResults;
    for (int i = 0; i < take; ++i) {
        results.add(scored.get(i).gig);
    }
    return results;
}

DataList<string> GigManager::autocompleteSuggestions(const string& prefix,
    int maxResults) const {
    string lower = Tokenizer::toLower(prefix);
    return trie_.findByPrefix(lower, maxResults);
}


void GigManager::rebuildIndexes() {
    index_.clear();
    trie_.clear();
    soundexIndex_.clear();

    DataList<Gig> all = gigRepo_->findAllActiveGigs();
    for (int i = 0; i < all.size(); ++i) {
        indexGig(all.get(i));
    }
}

int GigManager::getVocabularySize() const {
    return trie_.size();
}