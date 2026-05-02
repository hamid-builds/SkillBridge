#ifndef SKILLBRIDGE_TOKENIZER_H
#define SKILLBRIDGE_TOKENIZER_H

#include <string>
#include "DataList.h"





class Tokenizer {
public:
 
    static DataList<std::string> tokenize(const std::string& text);

   
    static bool isStopword(const std::string& lowercaseWord);

    
    static std::string toLower(const std::string& s);

    
    Tokenizer() = delete;
};

#endif