#include "utils.h"
#include <iostream>
std::string urlEncode(const std::string& SRC) {
    std::string ret;
    ret.reserve(SRC.size());
    for (std::string::const_iterator iter = SRC.begin(); iter != SRC.end(); ++iter) {
        std::string::value_type c = (*iter);
            // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            ret += c;
            continue;
        } else if(c == ' ') {
            ret+='+';
        }
        // Any other characters are percent-encoded
        char buff[4];
        sprintf(buff, "%%%02x", c);
        ret += buff;
    }
    return ret;
}
std::string urlEncode(const std::vector<uint8_t>& SRC) {
    std::string ret;
    ret.reserve(SRC.size());
    for (auto iter = SRC.begin(); iter != SRC.end(); ++iter) {
        uint8_t c = (*iter);
        char buff[4];
        sprintf(buff, "%%%02x", c);
        ret += buff;
    }
    return ret;
}
void fail(beast::error_code ec, char const *what){
std::cerr << what << ": " << ec.message() << "\n";
}