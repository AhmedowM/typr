#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <atomic>
#include <chrono>
#include <memory>
#include <string>

namespace typr {
class EngineBridge;
}

namespace typr::ui {

struct TypingScreenState {
    std::atomic<bool> errorFlashActive{false};
    std::chrono::steady_clock::time_point errorFlashStart;
    bool showIdleHint = true;
    std::string modeLabel = "STRICT";
    std::string timeoutLabel = "30s";
    bool showSettingsHints = true;
    std::function<void()> navigateToMain;
    std::function<void()> navigateToResults;
};

ftxui::Component TypingScreen(EngineBridge& engine, std::shared_ptr<TypingScreenState> state);

}

