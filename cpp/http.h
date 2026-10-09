#pragma once
#include "util.h"
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <vector>
#pragma comment(lib, "winhttp.lib")

struct HttpResponse { int status = 0; std::string body; };

inline HttpResponse http_request(
    const std::wstring& method,
    const std::wstring& host,
    const std::wstring& path,
    const std::string& body,
    const std::vector<std::wstring>& headers)
{
    HttpResponse res;

    HINTERNET hS = WinHttpOpen(L"Serpentine/1.0",
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if(!hS) return res;

    HINTERNET hC = WinHttpConnect(hS, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if(!hC){ WinHttpCloseHandle(hS); return res; }

    HINTERNET hR = WinHttpOpenRequest(hC, method.c_str(), path.c_str(),
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);
    if(!hR){
        WinHttpCloseHandle(hC);
        WinHttpCloseHandle(hS);
        return res;
    }

    for(auto& h : headers)
        WinHttpAddRequestHeaders(hR, h.c_str(), (ULONG)-1, WINHTTP_ADDREQ_FLAG_ADD);

    BOOL ok = WinHttpSendRequest(hR, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        (LPVOID)body.data(), (DWORD)body.size(), (DWORD)body.size(), 0);
    if(ok) ok = WinHttpReceiveResponse(hR, nullptr);

    if(ok){
        DWORD code = 0, sz = sizeof(code);
        WinHttpQueryHeaders(hR,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &code, &sz, WINHTTP_NO_HEADER_INDEX);
        res.status = (int)code;

        DWORD avail = 0;
        do {
            avail = 0;
            WinHttpQueryDataAvailable(hR, &avail);
            if(!avail) break;
            std::string buf(avail, 0);
            DWORD read = 0;
            WinHttpReadData(hR, &buf[0], avail, &read);
            res.body.append(buf.data(), read);
        } while(avail);
    }

    WinHttpCloseHandle(hR);
    WinHttpCloseHandle(hC);
    WinHttpCloseHandle(hS);
    return res;
}