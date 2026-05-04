#include "../../include/utils/PageRankCalculator.h"
#include <cmath>

PageRankCalculator::PageRankCalculator(double dampingFactor,
    int    maxIterations,
    double convergenceThreshold)
    : dampingFactor_(dampingFactor < 0.0 ? 0.0 :
        dampingFactor  > 1.0 ? 1.0 : dampingFactor),
    maxIterations_(maxIterations < 1 ? 1 : maxIterations),
    convergenceThreshold_(convergenceThreshold < 0.0 ? 0.0 : convergenceThreshold)
{
}

DataList<RankedUser> PageRankCalculator::calculate(const SkillGraph& graph) const {
    int N = graph.nodeCount();
    if (N == 0) return DataList<RankedUser>();

    DataList<int> nodeList = graph.nodes();

    DataList<double> rank;
    for (int i = 0; i < N; ++i)
        rank.add(1.0 / static_cast<double>(N));

    DataList<double> totalOut;
    for (int i = 0; i < N; ++i) {
        DataList<Endorsement> out = graph.getOutgoing(nodeList[i]);
        double sum = 0.0;
        for (int j = 0; j < out.size(); ++j)
            sum += out[j].getWeight();
        totalOut.add(sum);
    }

    DataList<double> newRank;
    for (int i = 0; i < N; ++i) newRank.add(0.0);

    for (int iter = 0; iter < maxIterations_; ++iter) {
        double danglingSum = 0.0;
        for (int i = 0; i < N; ++i)
            if (totalOut[i] == 0.0)
                danglingSum += rank[i];
        double danglingContrib = dampingFactor_ * danglingSum
            / static_cast<double>(N);
        double base = (1.0 - dampingFactor_) / static_cast<double>(N);

        for (int i = 0; i < N; ++i)
            newRank[i] = base + danglingContrib;

        for (int u = 0; u < N; ++u) {
            if (totalOut[u] == 0.0) continue;
            DataList<Endorsement> out = graph.getOutgoing(nodeList[u]);
            for (int e = 0; e < out.size(); ++e) {
                int toID = out[e].getToUserID();
                for (int v = 0; v < N; ++v) {
                    if (nodeList[v] == toID) {
                        newRank[v] += dampingFactor_
                            * rank[u]
                            * out[e].getWeight()
                            / totalOut[u];
                        break;
                    }
                }
            }
        }

        double maxDelta = 0.0;
        for (int i = 0; i < N; ++i) {
            double delta = std::fabs(newRank[i] - rank[i]);
            if (delta > maxDelta) maxDelta = delta;
        }
        for (int i = 0; i < N; ++i)
            rank[i] = newRank[i];

        if (maxDelta < convergenceThreshold_) break;
    }

    DataList<RankedUser> result;
    for (int i = 0; i < N; ++i) {
        RankedUser ru;
        ru.userID = nodeList[i];
        ru.score = rank[i];
        result.add(ru);
    }
    insertionSort(result);
    return result;
}

void PageRankCalculator::insertionSort(DataList<RankedUser>& list) {
    for (int i = 1; i < list.size(); ++i) {
        RankedUser key = list[i];
        int j = i - 1;
        while (j >= 0 && list[j].score < key.score) {
            list[j + 1] = list[j];
            --j;
        }
        list[j + 1] = key;
    }
}