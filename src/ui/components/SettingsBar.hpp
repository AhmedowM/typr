#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

namespace typr::ui {

struct SettingsBarConfig {
    std::string modeLabel = "STRICT";
    std::string timeoutLabel = "30s";
    bool showHints = true;  // true = idle, show (Ctrl+S) (Ctrl+T)
};

ftxui::Element SettingsBarElement(const SettingsBarConfig& config);

ftxui::Component SettingsBar(const SettingsBarConfig& config);

}