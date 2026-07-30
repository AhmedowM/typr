#pragma once

#include "ContentLocator.hpp"
#include "../engine/EngineBridge.hpp"
#include "../ui/components/ContentSelector.hpp"
#include "../ui/screens/MainScreen.hpp"
#include "../ui/screens/TypingScreen.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <atomic>
#include <memory>
#include <thread>
#include <vector>

namespace typr {

class Storage;

enum class ScreenType {
    Main,
    Typing,
    Results,
    History,
    Stats,
    Info
};

class AppController {
public:
    AppController(ftxui::ScreenInteractive& screen, Storage& storage);
    ~AppController();

    AppController(const AppController&) = delete;
    AppController& operator=(const AppController&) = delete;

    ftxui::Component root();

private:
    void navigateTo(ScreenType screen);
    void tickLoop();
    void startDownloadIfNeeded(const ContentPaths& paths);
    void applyProviderSelection();

    ftxui::ScreenInteractive& m_screen;
    EngineBridge m_engine;
    Storage* m_storage;

    int m_activeTab = 0;
    ftxui::Component m_root;

    ftxui::Component m_mainScreen;
    ftxui::Component m_typingScreen;
    ftxui::Component m_resultsScreen;
    ftxui::Component m_historyScreen;
    ftxui::Component m_statsScreen;
    ftxui::Component m_infoScreen;

    std::shared_ptr<ui::TypingScreenState> m_typingState;
    std::shared_ptr<ui::ContentSelectorState> m_contentState;

    ContentPaths m_contentPaths;

    std::vector<cpptypr::CallbackHandle> m_engineHandles;

    std::thread m_tickThread;
    std::atomic<bool> m_tickRunning{true};

    std::thread m_downloadThread;
    std::atomic<bool> m_downloadRunning{false};

    std::atomic<ScreenType> m_pendingNav{ScreenType::Main};
};

}
