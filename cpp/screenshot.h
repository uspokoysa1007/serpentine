#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#pragma comment(lib, "gdiplus.lib")

inline void ensure_dpi_aware(){
    SetProcessDpiAwarenessContext(
        DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
}

inline std::vector<uint8_t> screenshot_png(){
    using namespace Gdiplus;
    static bool init = false;
    static ULONG_PTR gdip_token = 0;
    if(!init){
        GdiplusStartupInput gsi;
        GdiplusStartup(&gdip_token, &gsi, nullptr);
        init = true;
    }

    int x  = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y  = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int cx = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int cy = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if(cx <= 0 || cy <= 0) return {};

    HDC hScreen = GetDC(nullptr);
    HDC hMem    = CreateCompatibleDC(hScreen);
    HBITMAP hBmp = CreateCompatibleBitmap(hScreen, cx, cy);
    HGDIOBJ old = SelectObject(hMem, hBmp);

    BitBlt(hMem, 0, 0, cx, cy, hScreen, x, y, SRCCOPY);

    Bitmap bmp(hBmp, nullptr);
    IStream* stream = nullptr;
    CreateStreamOnHGlobal(nullptr, TRUE, &stream);

    CLSID pngClsid;
    CLSIDFromString(L"{557cf406-1a04-11d3-9a73-0000f81ef32e}", &pngClsid);
    bmp.Save(stream, &pngClsid, nullptr);

    STATSTG st{};
    stream->Stat(&st, STATFLAG_NONAME);
    std::vector<uint8_t> out((size_t)st.cbSize.QuadPart);
    LARGE_INTEGER z{};
    stream->Seek(z, STREAM_SEEK_SET, nullptr);
    ULONG rd = 0;
    stream->Read(out.data(), (ULONG)out.size(), &rd);

    stream->Release();
    SelectObject(hMem, old);
    DeleteObject(hBmp);
    DeleteDC(hMem);
    ReleaseDC(nullptr, hScreen);
    return out;
}