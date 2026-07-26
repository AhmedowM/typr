#include "Theme.hpp"
#include <ftxui/dom/elements.hpp>

namespace typr::ui {

ftxui::Element header() {
    return ftxui::text("TYPR") | ftxui::bold | ftxui::color(ACCENT_CYAN) | ftxui::center
        | ftxui::bgcolor(BG_CARD) | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 1);
}

ftxui::Element footer(const std::vector<std::string>& hints) {
    std::string hintStr;
    for (size_t i = 0; i < hints.size(); ++i) {
        if (i > 0) hintStr += " | ";
        hintStr += hints[i];
    }
    return ftxui::text(hintStr) | ftxui::dim | ftxui::color(FOOTER_TEXT)
        | ftxui::bgcolor(BG_CARD) | ftxui::center
        | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 1);
}

}