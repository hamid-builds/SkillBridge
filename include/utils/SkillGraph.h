#ifndef SKILLBRIDGE_SKILLGRAPH_H
#define SKILLBRIDGE_SKILLGRAPH_H

#include "../core/Endorsement.h"
#include "DataList.h"




class SkillGraph {
public:
    SkillGraph() = default;
    ~SkillGraph() = default;

    SkillGraph(const SkillGraph&) = delete;
    SkillGraph& operator=(const SkillGraph&) = delete;

    void addEdge(const Endorsement& e);

   
    bool removeEdge(int endorsementID);

    DataList<Endorsement> getOutgoing(int fromUserID) const;

    DataList<Endorsement> getIncoming(int toUserID) const;

   
    int nodeCount() const;

   
    DataList<int> nodes() const;

    bool hasNode(int userID) const;

    void clear();

private:
    DataList<int>                   nodeIDs_;
    DataList<DataList<Endorsement>> outEdges_;
    DataList<DataList<Endorsement>> inEdges_;

    int indexOf(int userID) const;
    int indexOfOrInsert(int userID);
};

#endif