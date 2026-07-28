#pragma once

#include <filesystem>
#include <string>

namespace typr {

struct ContentPaths {
    std::filesystem::path sentencesDb;
    std::filesystem::path wordsDb;
    bool found = false; // true if both DBs exist at the resolved location
};

enum class DownloadState {
    Idle,
    Downloading,
    Ready,
    Failed
};

struct ContentLocator {
    /// Resolve DB paths by priority:
    ///   1. userDataDir() / "content"
    ///   2. exeDir() / "content"
    ///   3. exeDir()
    /// Returns the first where both DBs exist, or empty with found=false.
    static ContentPaths resolve();

    static std::string downloadUrlSentences();
    static std::string downloadUrlWords();

    /// Download a content DB file from ctypr release to the given path.
    /// Blocks the calling thread. Returns true on success.
    static bool download(const std::string& url, const std::filesystem::path& dest);

    /// Size of the DB for progress estimation (bytes from Content-Length).
    /// Returns 0 if unknown.
    static size_t expectedSize(const std::string& url);
};

}
