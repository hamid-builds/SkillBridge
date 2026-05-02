#include "utils/SoundexEncoder.h"
#include <cctype>

using namespace std;

char SoundexEncoder::digitFor(char c) {
   
    switch (c) {
    case 'B': case 'F': case 'P': case 'V':
        return '1';
    case 'C': case 'G': case 'J': case 'K':
    case 'Q': case 'S': case 'X': case 'Z':
        return '2';
    case 'D': case 'T':
        return '3';
    case 'L':
        return '4';
    case 'M': case 'N':
        return '5';
    case 'R':
        return '6';
       
    default:
        return '0';
    }
}

string SoundexEncoder::encode(const string& word) {
  
    size_t i = 0;
    while (i < word.size() && !isalpha(static_cast<unsigned char>(word[i]))) {
        ++i;
    }

    
    if (i >= word.size()) {
        return "0000";
    }

   
    string result;
    char firstLetter = static_cast<char>(toupper(static_cast<unsigned char>(word[i])));
    result.push_back(firstLetter);

   
    char prevDigit = digitFor(firstLetter);

    ++i;

   
    for (; i < word.size() && result.size() < 4; ++i) {
        char c = word[i];
        if (!isalpha(static_cast<unsigned char>(c))) {
            
            continue;
        }
        char up = static_cast<char>(toupper(static_cast<unsigned char>(c)));

       
        if (up == 'H' || up == 'W') {
            continue;
        }

        char d = digitFor(up);

        if (d == '0') {
           
            prevDigit = '0';
            continue;
        }

       
        if (d == prevDigit) {
            continue;
        }

        result.push_back(d);
        prevDigit = d;
    }

    
    while (result.size() < 4) {
        result.push_back('0');
    }

    return result;
}