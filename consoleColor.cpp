//
// Created by Max on 31. 7. 2021.
//
#include <string>
#include "consoleColor.h"

std::ostream& operator<<(std::ostream& os, const color& color)
{
    std::string str("\033[");
    str += std::to_string(static_cast<int>(color.fg));
    str += ';';
    str += std::to_string(static_cast<int>(color.bg));
    str += 'm';
    os << str;//<<  <<static_cast<int>(color.fg) << ';' << static_cast<int>(color.bg) << 'm';
    return os;
}