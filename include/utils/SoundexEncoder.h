#ifndef SKILLBRIDGE_SOUNDEX_ENCODER_H
#define SKILLBRIDGE_SOUNDEX_ENCODER_H

#include <string>






class SoundexEncoder {
public:
    
    static std::string encode(const std::string& word);

    
    SoundexEncoder() = delete;

private:
   
    static char digitFor(char upperLetter);
};

#endif