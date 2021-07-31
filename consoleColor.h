//
// Created by Max on 31. 7. 2021.
//

#ifndef STEAMBOT_CONSOLECOLOR_H
#define STEAMBOT_CONSOLECOLOR_H
#include <iostream>
#ifdef WIN32
#include <WinCon.h>
#endif

enum class colorFG {
    Black = 30,
    Red = 31, //FOREGROUND_RED
    Green = 32, //FOREGROUND_GREEN
    Yellow = 33,
    Blue = 34, //FOREGROUND_BLUE
    Magenta = 35,
    Cyan = 36,
    White = 37,
    Bright_Black = 90,      // FOREGROUND_INTENSITY |
    Bright_Red = 91,        // FOREGROUND_INTENSITY |
    Bright_Green = 92,      // FOREGROUND_INTENSITY |
    Bright_Yellow = 93,     // FOREGROUND_INTENSITY |
    Bright_Blue = 94,       // FOREGROUND_INTENSITY |
    Bright_Magenta = 95,    // FOREGROUND_INTENSITY |
    Bright_Cyan = 96,       // FOREGROUND_INTENSITY |
    Bright_White = 97,      // FOREGROUND_INTENSITY |
    Default = 39

};
enum class  colorBG {
    Black = 40,
    Red = 41,
    Green = 42,
    Yellow = 43,
    Blue = 44,
    Magenta = 45,
    Cyan = 46,
    White = 47,
    Bright_Black = 100,
    Bright_Red = 101,
    Bright_Green = 102,
    Bright_Yellow = 103,
    Bright_Blue = 104,
    Bright_Magenta = 105,
    Bright_Cyan = 106,
    Bright_White = 107,
    Default = 49
};
struct color {
    colorFG fg;
    colorBG bg;
    explicit color(colorFG fgc) {
        fg = fgc;
        bg = colorBG::Default;
    };
    explicit color(colorBG bgc) {
        bg = bgc;
        fg = colorFG::Default;
    };
    color(colorFG fgc, colorBG bgc) : fg(fgc), bg(bgc) {};
    color() : fg(colorFG::Default), bg(colorBG::Default) {};
};
//TODO: REMOVE inline and do it properly

std::ostream& operator<<(std::ostream& os, const color& color);
#endif //STEAMBOT_CONSOLECOLOR_H
