#pragma once
#include <windows.h>
#include <filesystem>
#include <vector>
#include <string>
namespace fs = std::filesystem;

inline std::vector<std::string> list_users(){
    std::vector<std::string> out;
    std::error_code ec;
    if(!fs::exists("C:\\Users", ec)) return out;
    for(auto& p : fs::directory_iterator("C:\\Users", ec)){
        if(ec){ ec.clear(); continue; }
        if(!p.is_directory()) continue;
        auto name = p.path().filename().string();
        if(name == "Public" || name == "Default" ||
           name == "Default User" || name == "All Users") continue;
        out.push_back(name);
    }
    return out;
}

inline bool can_read(const std::string& path){
    DWORD a = GetFileAttributesA(path.c_str());
    if(a == INVALID_FILE_ATTRIBUTES) return false;
    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    if(h == INVALID_HANDLE_VALUE) return false;
    CloseHandle(h);
    return true;
}