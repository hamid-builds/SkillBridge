#ifndef SKILLBRIDGE_SEED_DATA_H
#define SKILLBRIDGE_SEED_DATA_H

#include "managers/IGigRepository.h"



class SeedData {
public:
  
    static int seedGigs(IGigRepository& repo,
        const DataList<int>& freelancerIDs);

    SeedData() = delete;
};

#endif