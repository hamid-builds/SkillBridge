#ifndef SKILLBRIDGE_PAGERANKCALCULATOR_H
#define SKILLBRIDGE_PAGERANKCALCULATOR_H

#include "SkillGraph.h"
#include "DataList.h"



struct RankedUser {
    int    userID;
    double score;
    bool operator<(const RankedUser& o) const { return score > o.score; }
};

class PageRankCalculator {
public:
    explicit PageRankCalculator(double dampingFactor = 0.85,
        int    maxIterations = 100,
        double convergenceThreshold = 1e-6);

    DataList<RankedUser> calculate(const SkillGraph& graph) const;

private:
    double dampingFactor_;
    int    maxIterations_;
    double convergenceThreshold_;

    static void insertionSort(DataList<RankedUser>& list);
};

#endif