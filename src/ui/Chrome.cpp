#include "Chrome.hpp"
#include "Theme.hpp"
#include <ftxui/dom/elements.hpp>

namespace typr::ui {

ftxui::Element Header(const ftxui::Element& content) {
    return ftxui::hbox({
        ftxui::filler(),
        content | ftxui::bold | ftxui::color(ACCENT_CYAN) | ftxui::center,
        ftxui::filler(),
    }) | ftxui::bgcolor(BG_CARD) | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 1);
}

ftxui::Element Footer(const ftxui::Element& hints) {
    return ftxui::hbox({
        ftxui::filler(),
        hints | ftxui::dim | ftxui::color(DIM_GRAY_1) | ftxui::center,
        ftxui::filler(),
    }) | ftxui::bgcolor(BG_CARD) | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 1);
}

}