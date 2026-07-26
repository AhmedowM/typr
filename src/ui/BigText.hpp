#pragma once

#include <ftxui/dom/elements.hpp>
#include <string>
#include <unordered_map>
#include <array>

namespace typr::ui {

class BigText {
public:
    static ftxui::Element render(const std::string& text, ftxui::Color color = ftxui::Color::Default, bool bold = true);

private:
    static const std::unordered_map<char, std::array<std::string, 4>>& glyphMap();
};

}