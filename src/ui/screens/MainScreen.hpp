#pragma once

#include "../components/ContentSelector.hpp"
#include <ftxui/component/component.hpp>
#include <functional>
#include <memory>

namespace typr::ui {

enum class MainAction {
    StartPractice,
    History,
    Stats,
    Info,
    Quit
};

struct MainScreenCallbacks {
    std::function<void()> onStartPractice;
    std::function<void()> onHistory;
    std::function<void()> onStats;
    std::function<void()> onInfo;
    std::function<void()> onQuit;
};

ftxui::Component MainScreen(const MainScreenCallbacks& callbacks,
                            std::shared_ptr<ContentSelectorState> contentState);

}