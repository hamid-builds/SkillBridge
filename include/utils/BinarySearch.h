#ifndef SKILLBRIDGE_BINARYSEARCH_H
#define SKILLBRIDGE_BINARYSEARCH_H

#include "DataList.h"




template <typename T, typename Compare>
int binarySearch(const DataList<T>& list, const T& target, Compare cmp) {
    int lo = 0;
    int hi = list.size();  

    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (cmp(list[mid], target)) {
            lo = mid + 1;              
        }
        else if (cmp(target, list[mid])) {
            hi = mid;                
        }
        else {
            return mid;                
        }
    }
    return -1;
}


template <typename T, typename Compare>
int lowerBound(const DataList<T>& list, const T& target, Compare cmp) {
    int lo = 0;
    int hi = list.size();

    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (cmp(list[mid], target)) {
            lo = mid + 1;             
        }
        else {
            hi = mid;                  
        }
    }
    return lo;
}

template <typename T, typename Compare>
int upperBound(const DataList<T>& list, const T& target, Compare cmp) {
    int lo = 0;
    int hi = list.size();

    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (!cmp(target, list[mid])) {
            lo = mid + 1;             
        }
        else {
            hi = mid;                 
        }
    }
    return lo;
}

#endif