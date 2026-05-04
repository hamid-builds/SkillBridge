#include "../../include/utils/GraphTraverser.h"

struct BFSEntry {
    int userID;
    int depth;
};

GraphTraverser::GraphTraverser(int maxHops)
    : maxHops_(maxHops < 1 ? 1 : maxHops)
{
}

DataList<TrustedUser> GraphTraverser::findTrusted(const SkillGraph& graph,
    int startUserID) const {
    DataList<TrustedUser> result;
    if (!graph.hasNode(startUserID)) return result;

    DataList<BFSEntry> queue;
    DataList<int>      visitedIDs;

    BFSEntry seed;
    seed.userID = startUserID;
    seed.depth = 0;
    queue.add(seed);
    visitedIDs.add(startUserID);

    int head = 0;

    while (head < queue.size()) {
        BFSEntry current = queue[head++];

        if (current.depth >= maxHops_) continue;

        DataList<Endorsement> outgoing = graph.getOutgoing(current.userID);
        for (int i = 0; i < outgoing.size(); ++i) {
            int neighbor = outgoing[i].getToUserID();

            bool seen = false;
            for (int j = 0; j < visitedIDs.size(); ++j)
                if (visitedIDs[j] == neighbor) { seen = true; break; }
            if (seen) continue;

            visitedIDs.add(neighbor);

            TrustedUser tu;
            tu.userID = neighbor;
            tu.hopCount = current.depth + 1;
            result.add(tu);

            if (current.depth + 1 < maxHops_) {
                BFSEntry next;
                next.userID = neighbor;
                next.depth = current.depth + 1;
                queue.add(next);
            }
        }
    }

    sort(result);
    return result;
}

bool GraphTraverser::visited(const DataList<TrustedUser>& seen, int userID) {
    for (int i = 0; i < seen.size(); ++i)
        if (seen[i].userID == userID) return true;
    return false;
}

void GraphTraverser::sort(DataList<TrustedUser>& list) {
    for (int i = 1; i < list.size(); ++i) {
        TrustedUser key = list[i];
        int j = i - 1;
        while (j >= 0) {
            bool shouldSwap =
                (list[j].hopCount > key.hopCount) ||
                (list[j].hopCount == key.hopCount && list[j].userID > key.userID);
            if (!shouldSwap) break;
            list[j + 1] = list[j];
            --j;
        }
        list[j + 1] = key;
    }
}