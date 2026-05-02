#ifndef SKILLBRIDGE_SORTING_H
#define SKILLBRIDGE_SORTING_H

#include "DataList.h"


namespace sorting_detail {

    template <typename T, typename Compare>
    void merge(DataList<T>& list, DataList<T>& buffer,
        int lo, int mid, int hi, Compare cmp) {
       
        for (int i = lo; i < hi; ++i) {
            buffer[i] = list[i];
        }

        int i = lo;   
        int j = mid;   
        int k = lo;   

       
        while (i < mid && j < hi) {
            if (!cmp(buffer[j], buffer[i])) {
                list[k++] = buffer[i++];
            }
            else {
                list[k++] = buffer[j++];
            }
        }

       
        while (i < mid) list[k++] = buffer[i++];
        while (j < hi)  list[k++] = buffer[j++];
    }

    template <typename T, typename Compare>
    void mergeSortHelper(DataList<T>& list, DataList<T>& buffer,
        int lo, int hi, Compare cmp) {
        if (hi - lo <= 1) return;

        int mid = lo + (hi - lo) / 2;
        mergeSortHelper(list, buffer, lo, mid, cmp);
        mergeSortHelper(list, buffer, mid, hi, cmp);
        merge(list, buffer, lo, mid, hi, cmp);
    }

} 


template <typename T, typename Compare>
void mergeSort(DataList<T>& list, Compare cmp) {
    int n = list.size();
    if (n <= 1) return;

   
    DataList<T> buffer;
    for (int i = 0; i < n; ++i) {
        buffer.add(list[i]);
    }

    sorting_detail::mergeSortHelper(list, buffer, 0, n, cmp);
}

#endif