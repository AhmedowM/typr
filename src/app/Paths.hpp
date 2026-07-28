#pragma once

#include <cstdlib>
#include <filesystem>

namespace typr {

struct Paths {
    static std::filesystem::path userDataDir() {
#ifdef _WIN32
        const char* appData = std::getenv("APPDATA");
        if (appData && appData[0] != '\0') {
            return std::filesystem::path(appData) / "typr";
        }
        return std::filesystem::path(std::getenv("USERPROFILE")) / "AppData" / "Roaming" / "typr";
#else
        const char* xdgData = std::getenv("XDG_DATA_HOME");
        if (xdgData && xdgData[0] != '\0') {
            return std::filesystem::path(xdgData) / "typr";
        }
        const char* home = std::getenv("HOME");
        if (home && home[0] != '\0') {
            return std::filesystem::path(home) / ".local" / "share" / "typr";
        }
        return std::filesystem::path("/tmp") / "typr";
#endif
    }

    static std::filesystem::path sessionsDb() {
        auto dir = userDataDir();
        return dir / "sessions.db";
    }

    static std::filesystem::path contentDbDir() {
        return userDataDir() / "content";
    }

    static void ensureDataDir() {
        std::filesystem::create_directories(userDataDir());
    }
};

}
