#pragma once

#include <ftxui/component/component.hpp>
#include <functional>

namespace typr::ui {

enum class MainAction {
    StartPractice,
    History,
    Stats,
    Quit
};

struct MainScreenCallbacks {
    std::function<void()> onStartPractice;
    std::function<void()> onHistory;
    std::function<void()> onStats;
    std::function<void()> onQuit;
};

ftxui::Component MainScreen(const MainScreenCallbacks& callbacks);

}