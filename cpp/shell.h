#pragma once
#include "util.h"
#include <windows.h>
#include <string>

inline std::string run_process(const std::wstring& cmdline,
                               DWORD timeoutMs = 15000){
    SECURITY_ATTRIBUTES sa{ sizeof(sa), nullptr, TRUE };
    HANDLE r = nullptr, w = nullptr;
    if(!CreatePipe(&r, &w, &sa, 0)) return "";
    SetHandleInformation(r, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = w;
    si.hStdError  = w;
    si.hStdInput  = nullptr;

    PROCESS_INFORMATION pi{};
    std::wstring c = cmdline;
    BOOL ok = CreateProcessW(nullptr, &c[0], nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    CloseHandle(w);
    if(!ok){ CloseHandle(r); return ""; }

    WaitForSingleObject(pi.hProcess, timeoutMs);

    std::string out;
    char buf[4096];
    DWORD rd = 0;
    while(ReadFile(r, buf, sizeof(buf), &rd, nullptr) && rd > 0)
        out.append(buf, rd);

    CloseHandle(r);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return out;
}

inline std::string run_cmd(const std::string& cmd){
    std::wstring wcmd = L"cmd.exe /c " + to_w(cmd);
    return run_process(wcmd);
}

inline std::string run_ps(const std::string& cmd){
    std::wstring wcmd = L"powershell.exe -NoProfile -NonInteractive -Command \""
                      + to_w(cmd) + L"\"";
    return run_process(wcmd);
}