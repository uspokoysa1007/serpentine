#pragma once
#include <windows.h>
#include <shellapi.h>
#include <string>
#pragma comment(lib, "shell32.lib")

inline bool is_admin(){
    BOOL admin = FALSE;
    PSID grp = nullptr;
    SID_IDENTIFIER_AUTHORITY nt = SECURITY_NT_AUTHORITY;
    if(AllocateAndInitializeSid(&nt, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0,0,0,0,0,0, &grp)){
        CheckTokenMembership(nullptr, grp, &admin);
        FreeSid(grp);
    }
    return admin == TRUE;
}

inline std::string whoami(){
    char u[256] = {0};
    DWORD n = sizeof(u);
    GetUserNameA(u, &n);
    return std::string(u);
}

inline std::string hostname(){
    char h[256] = {0};
    DWORD n = sizeof(h);
    GetComputerNameA(h, &n);
    return std::string(h);
}