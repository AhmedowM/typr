#include "SettingsBar.hpp"
#include "../Theme.hpp"
#include <ftxui/dom/elements.hpp>

namespace typr::ui {

ftxui::Element SettingsBarElement(const SettingsBarConfig& config) {
    std::string modeText = "MODE: [" + config.modeLabel + "]";
    std::string timeoutText = "TIMEOUT: [" + config.timeoutLabel + "]";

    if (config.showHints) {
        modeText += " (Ctrl+S)";
        timeoutText += " (Ctrl+T)";
    }

    return ftxui::hbox({
        ftxui::text(modeText) | ftxui::color(ColorLabel) | ftxui::center | ftxui::flex,
        ftxui::text(timeoutText) | ftxui::color(ColorLabel) | ftxui::center | ftxui::flex,
    }) | ftxui::borderRounded | ftxui::color(ColorBorder)
      | ftxui::bgcolor(ColorCardBackground)
      | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 3);
}

ftxui::Component SettingsBar(const SettingsBarConfig& config) {
    return ftxui::Renderer([=] {
        return SettingsBarElement(config);
    });
}

}