#pragma once
#include "util.h"
#include "../third_party/miniz.h"
#include <filesystem>
#include <vector>
#include <string>
#include <windows.h>
namespace fs = std::filesystem;

inline void zip_add_dir(mz_zip_archive* zip, const std::string& src,
                        const std::string& prefix = ""){
    std::error_code ec;
    if(!fs::exists(src, ec)) return;
    for(auto& p : fs::recursive_directory_iterator(src,
            fs::directory_options::skip_permission_denied, ec)){
        if(ec){ ec.clear(); continue; }
        if(!p.is_regular_file(ec)) continue;
        auto path = p.path().string();
        std::string rel = fs::relative(p.path(), src, ec).string();
        std::string entry = prefix.empty() ? rel : (prefix + "/" + rel);
        try {
            mz_zip_writer_add_file(zip, entry.c_str(), path.c_str(),
                nullptr, 0, MZ_DEFAULT_COMPRESSION);
        } catch(...) {}
    }
}

inline bool zip_dir(const std::string& src, const std::string& dst){
    mz_zip_archive zip{};
    if(!mz_zip_writer_init_file(&zip, dst.c_str(), 0)) return false;
    zip_add_dir(&zip, src);
    mz_zip_writer_finalize_archive(&zip);
    mz_zip_writer_end(&zip);
    return true;
}

inline std::vector<uint8_t> read_file(const std::string& path){
    std::vector<uint8_t> out;
    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                           nullptr, OPEN_EXISTING, 0, nullptr);
    if(h == INVALID_HANDLE_VALUE) return out;
    DWORD sz = GetFileSize(h, nullptr);
    if(sz == INVALID_FILE_SIZE){ CloseHandle(h); return out; }
    out.resize(sz);
    DWORD rd = 0;
    ReadFile(h, out.data(), sz, &rd, nullptr);
    out.resize(rd);
    CloseHandle(h);
    return out;
}