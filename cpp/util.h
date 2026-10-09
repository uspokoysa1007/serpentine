#pragma once
#include <windows.h>
#include <string>

inline std::wstring to_w(const std::string& s){
    if(s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring r(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &r[0], n);
    return r;
}

inline std::string to_a(const std::wstring& s){
    if(s.empty()) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0, nullptr, nullptr);
    std::string r(n, 0);
    WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), &r[0], n, nullptr, nullptr);
    return r;
}

inline std::string url_encode(const std::string& s){
    static const char* hex = "0123456789ABCDEF";
    std::string out;
    for(unsigned char c : s){
        if(isalnum(c) || c=='-' || c=='_' || c=='.' || c=='~') out += (char)c;
        else { out += '%'; out += hex[c>>4]; out += hex[c&15]; }
    }
    return out;
}