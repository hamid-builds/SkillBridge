#ifndef SKILLBRIDGE_GRAPHTRAVERSER_H
#define SKILLBRIDGE_GRAPHTRAVERSER_H

#include "SkillGraph.h"
#include "DataList.h"




struct TrustedUser {
    int userID;
    int hopCount;
};

class GraphTraverser {
public:
    explicit GraphTraverser(int maxHops = 2);

   
    DataList<TrustedUser> findTrusted(const SkillGraph& graph,
        int startUserID) const;

private:
    int maxHops_;

    static bool visited(const DataList<TrustedUser>& seen, int userID);
    static void sort(DataList<TrustedUser>& list);
};

#endif