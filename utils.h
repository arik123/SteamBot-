#ifndef STEAMBOT_UTILS_H
#define STEAMBOT_UTILS_H
#include <string>
#include <boost/beast/core.hpp>
namespace beast = boost::beast;
std::string urlEncode(const std::string& SRC);
std::string urlEncode(const std::vector<uint8_t>& SRC);
void fail(beast::error_code ec, char const *what);
#endif