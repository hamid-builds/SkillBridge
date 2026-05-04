#include "../../include/utils/SkillGraph.h"

int SkillGraph::indexOf(int userID) const {
    for (int i = 0; i < nodeIDs_.size(); ++i)
        if (nodeIDs_[i] == userID) return i;
    return -1;
}

int SkillGraph::indexOfOrInsert(int userID) {
    int idx = indexOf(userID);
    if (idx != -1) return idx;
    nodeIDs_.add(userID);
    outEdges_.add(DataList<Endorsement>());
    inEdges_.add(DataList<Endorsement>());
    return nodeIDs_.size() - 1;
}

void SkillGraph::addEdge(const Endorsement& e) {
    int fromIdx = indexOf(e.getFromUserID());
    if (fromIdx != -1) {
        DataList<Endorsement>& out = outEdges_[fromIdx];
        for (int i = 0; i < out.size(); ++i)
            if (out[i].getEndorsementID() == e.getEndorsementID()) return;
    }
    int fi = indexOfOrInsert(e.getFromUserID());
    int ti = indexOfOrInsert(e.getToUserID());
    outEdges_[fi].add(e);
    inEdges_[ti].add(e);
}

bool SkillGraph::removeEdge(int endorsementID) {
    bool found = false;
    for (int i = 0; i < nodeIDs_.size() && !found; ++i) {
        DataList<Endorsement>& out = outEdges_[i];
        for (int j = 0; j < out.size(); ++j) {
            if (out[j].getEndorsementID() == endorsementID) {
                DataList<Endorsement> rebuilt;
                for (int k = 0; k < out.size(); ++k)
                    if (k != j) rebuilt.add(out[k]);
                out = rebuilt;
                found = true;
                break;
            }
        }
    }
    if (!found) return false;

    for (int i = 0; i < nodeIDs_.size(); ++i) {
        DataList<Endorsement>& in = inEdges_[i];
        for (int j = 0; j < in.size(); ++j) {
            if (in[j].getEndorsementID() == endorsementID) {
                DataList<Endorsement> rebuilt;
                for (int k = 0; k < in.size(); ++k)
                    if (k != j) rebuilt.add(in[k]);
                in = rebuilt;
                break;
            }
        }
    }
    return true;
}

DataList<Endorsement> SkillGraph::getOutgoing(int fromUserID) const {
    int idx = indexOf(fromUserID);
    if (idx == -1) return DataList<Endorsement>();
    return outEdges_[idx];
}

DataList<Endorsement> SkillGraph::getIncoming(int toUserID) const {
    int idx = indexOf(toUserID);
    if (idx == -1) return DataList<Endorsement>();
    return inEdges_[idx];
}

int SkillGraph::nodeCount() const {
    return nodeIDs_.size();
}

DataList<int> SkillGraph::nodes() const {
    return nodeIDs_;
}

bool SkillGraph::hasNode(int userID) const {
    return indexOf(userID) != -1;
}

void SkillGraph::clear() {
    nodeIDs_ = DataList<int>();
    outEdges_ = DataList<DataList<Endorsement>>();
    inEdges_ = DataList<DataList<Endorsement>>();
}