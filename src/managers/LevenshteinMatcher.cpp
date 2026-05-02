#include "managers/LevenshteinMatcher.h"
#include <vector>
#include <algorithm>

using namespace std;

int LevenshteinMatcher::distance(const string& a, const string& b) const {
    int n = static_cast<int>(a.size());
    int m = static_cast<int>(b.size());

    
    if (n == 0) return m;
    if (m == 0) return n;

    
    if (n < m) {
        return distance(b, a);
    }

    
    vector<int> prev(m + 1);
    vector<int> curr(m + 1);

   
    for (int j = 0; j <= m; ++j) {
        prev[j] = j;
    }

    for (int i = 1; i <= n; ++i) {
        
        curr[0] = i;

        for (int j = 1; j <= m; ++j) {
            if (a[i - 1] == b[j - 1]) {
               
                curr[j] = prev[j - 1];
            }
            else {
               
                int del = prev[j];         
                int ins = curr[j - 1];     
                int sub = prev[j - 1];     
                curr[j] = 1 + min({ del, ins, sub });
            }
        }

       
        swap(prev, curr);
    }

   
    return prev[m];
}