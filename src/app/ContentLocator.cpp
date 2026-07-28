#include "ContentLocator.hpp"
#include "Paths.hpp"

#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")
#else
#include <unistd.h>
#endif

namespace typr {

static std::filesystem::path exeDir() {
    std::vector<char> buf(4096);
#ifdef _WIN32
    GetModuleFileNameA(NULL, buf.data(), static_cast<DWORD>(buf.size()));
#else
    ssize_t len = readlink("/proc/self/exe", buf.data(), buf.size() - 1);
    if (len > 0) buf[len] = '\0';
#endif
    return std::filesystem::path(buf.data()).parent_path();
}

ContentPaths ContentLocator::resolve() {
    auto check = [](const std::filesystem::path& dir) -> ContentPaths {
        auto s = dir / "sentences.db";
        auto w = dir / "words.db";
        if (exists(s) && exists(w))
            return {s, w, true};
        return {s, w, false};
    };

    auto result = check(Paths::contentDbDir());
    if (result.found) return result;

    result = check(exeDir() / "content");
    if (result.found) return result;

    result = check(exeDir());
    if (result.found) return result;

    return {Paths::contentDbDir() / "sentences.db",
            Paths::contentDbDir() / "words.db", false};
}

std::string ContentLocator::downloadUrlSentences() {
    return "https://github.com/AhmedowM/ctypr/releases/download/"
           TYPR_CONTENT_VERSION "/sentences.db";
}

std::string ContentLocator::downloadUrlWords() {
    return "https://github.com/AhmedowM/ctypr/releases/download/"
           TYPR_CONTENT_VERSION "/words.db";
}

bool ContentLocator::download(const std::string& url, const std::filesystem::path& dest) {
    auto dir = dest.parent_path();
    if (!exists(dir))
        create_directories(dir);

#ifdef _WIN32
    HRESULT hr = URLDownloadToFileA(NULL, url.c_str(), dest.string().c_str(), 0, NULL);
    return SUCCEEDED(hr);
#else
    std::string cmd = "curl -L -s -o \"" + dest.string() + "\" \"" + url + "\"";
    int rc = std::system(cmd.c_str());
    return rc == 0;
#endif
}

size_t ContentLocator::expectedSize(const std::string& url) {
    (void)url;
    return 0;
}

}
