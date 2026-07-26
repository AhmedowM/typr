#pragma once

#include "engine/EngineBridge.hpp"
#include "ui/screens/MainScreen.hpp"
#include "ui/screens/TypingScreen.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>

namespace typr {

class Storage;

enum class ScreenType {
    Main,
    Typing,
    Results,
    History,
    Stats
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

    std::shared_ptr<ui::TypingScreenState> m_typingState;

    std::vector<cpptypr::CallbackHandle> m_engineHandles;

    std::thread m_tickThread;
    std::atomic<bool> m_tickRunning{true};

    std::atomic<ScreenType> m_pendingNav{ScreenType::Main};
};

}
