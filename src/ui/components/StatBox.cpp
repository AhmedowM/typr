#include "StatBox.hpp"
#include "../Theme.hpp"
#include <ftxui/dom/elements.hpp>

namespace typr::ui {

ftxui::Element StatBoxElement(const std::string& label, const std::string& value, ftxui::Color valueColor) {
    return ftxui::vbox({
        ftxui::text(value) | ftxui::bold | ftxui::color(valueColor) | ftxui::center,
        ftxui::text(label) | ftxui::italic | ftxui::color(ColorLabelDim) | ftxui::center,
    }) | ftxui::borderRounded | ftxui::color(ColorBorder) | ftxui::flex | ftxui::center;
}

ftxui::Component StatBox(const std::string& label, const std::string& value, ftxui::Color valueColor) {
    return ftxui::Renderer([=] {
        return StatBoxElement(label, value, valueColor);
    });
}

}