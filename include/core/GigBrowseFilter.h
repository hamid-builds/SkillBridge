#ifndef SKILLBRIDGE_GIGBROWSEFILTER_H
#define SKILLBRIDGE_GIGBROWSEFILTER_H

#include "GigCategory.h"

struct GigBrowseFilter 
{
    bool hasCategory = false;
    GigCategory category = GigCategory::OTHER;
};

enum class GigSortOrder 
{
    NEWEST_FIRST, PRICE_ASC, PRICE_DESC
};

#endif