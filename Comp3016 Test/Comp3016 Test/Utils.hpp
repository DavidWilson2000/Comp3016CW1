#pragma once
#include <string>
#include <algorithm>
#include <cctype>


inline std::string trim(std::string s) {
    auto notspace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notspace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notspace).base(), s.end());
    return s;
}


//  Console color helpers 

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
enum class ConsoleColor : WORD {
    Default = 7, Red = 12, Green = 10,
    Yellow = 14, Cyan = 11, White = 15, Gray = 8
};
inline HANDLE _consoleHandle() {
    static HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    return h;
}
inline void setColor(ConsoleColor c) {
    SetConsoleTextAttribute(_consoleHandle(), static_cast<WORD>(c));
}
inline void resetColor() { setColor(ConsoleColor::Default); }
#else
enum class ConsoleColor {
    Default, Red, Green, Yellow, Cyan, White, Gray
};
inline void setColor(ConsoleColor c) {
    switch (c) {
    case ConsoleColor::Red:    printf("\033[91m"); break;
    case ConsoleColor::Green:  printf("\033[92m"); break;
    case ConsoleColor::Yellow: printf("\033[93m"); break;
    case ConsoleColor::Cyan:   printf("\033[96m"); break;
    case ConsoleColor::White:  printf("\033[97m"); break;
    case ConsoleColor::Gray:   printf("\033[90m"); break;
    default:                   printf("\033[0m");  break;
    }
}
inline void resetColor() { printf("\033[0m"); }
#endif
