#include "BigText.hpp"
#include <ftxui/dom/elements.hpp>
#include <unordered_map>
#include <array>
#include <string>
#include <algorithm>

namespace typr::ui {

namespace {
    const std::unordered_map<char, std::array<std::string, 4>>& glyphMapImpl() {
        static const std::unordered_map<char, std::array<std::string, 4>> map = {
            { 'a', { "    ", "█▀▀█", "█▄ █", "    " }},
            { 'b', {"█   ", "█▀▀█", "█▄▄█", "    " }},
            { 'c', {"   ", "█▀▀", "█▄▄", "   " }},
            { 'd', {"   █", "█▀▀█", "█▄▄█", "    " }},
            { 'e', {"    ", "█▀▀█", "█▄▄ ", "    " }},
            { 'f', {"   ", "█▀▀", "█▀ ", "   " }},
            { 'g', {"    ", "█▀▀█", "█▄▄█", " ▄▄█" }},
            { 'h', {"█   ", "█▀▀█", "█  █", "    " }},
            { 'i', {"▀", "▄", "█", " " }},
            { 'j', {" ▀", " ▄", " █", "▄█" }},
            { 'k', {"█   ", "█▄█▀", "█  █", "    " }},
            { 'l', {"█", "█", "█", " " }},
            { 'm', {"     ", "█▀█▀█", "█ █ █", "     " }},
            { 'n', {"    ", "█▀▀█", "█  █", "    " }},
            { 'o', {"    ", "█▀▀█", "█▄▄█", "    " }},
            { 'p', {"    ", "█▀▀█", "█▄▄█", "█   " }},
            { 'q', {"    ", "█▀▀█", "█▄▄█", "   █" }},
            { 'r', {"   ", "█▀▀", "█  ", "   " }},
            { 's', {"   ", "█▀▀", "▄▄█", "   " }},
            { 't', {"█  ", "█▀ ", "█▄▄", "   " }},
            { 'u', {"    ", "█  █", "█▄▄█", "    " }},
            { 'v', {"    ", "█  █", "▀▄▄▀", "    " }},
            { 'w', {"     ", "█ █ █", "█▄█▄█", "     " }},
            { 'x', {"    ", "▀ ▄▀", "▄▀ ▄", "    " }},
            { 'y', {"    ", "█  █", "█▄▄█", " ▄▄█" }},
            { 'z', {"   ", "▀▀█", "█▄▄", "   " }},
            { '!', {"█", "█", " ", "▀" }},
            { ' ', {"  ", "  ", "  ", "  " }}
        };
        
        return map;
    }
}

const std::unordered_map<char, std::array<std::string, 4>>& BigText::glyphMap() {
    return glyphMapImpl();
}

ftxui::Element BigText::render(const std::string& text, ftxui::Color color, bool bold) {
    std::array<std::string, 4> rows = {"", "", "", ""};

    for (size_t i = 0; i < text.size(); ++i) {
        char lower = std::tolower(static_cast<unsigned char>(text[i]));
        auto it = glyphMap().find(lower);
        if (it == glyphMap().end()) {
            it = glyphMap().find(' ');
        }
        const auto& glyph = it->second;
        for (int r = 0; r < 4; ++r) {
            rows[r] += glyph[r];
            if (i + 1 < text.size()) {
                rows[r] += " ";
            }
        }
    }

    ftxui::Element result = ftxui::emptyElement();
    for (int r = 0; r < 4; ++r) {
        ftxui::Element line = ftxui::text(rows[r]);
        if (color != ftxui::Color::Default) line |= ftxui::color(color);
        if (bold) line |= ftxui::bold;
        if (r == 0) {
            result = line;
        } else {
            result = ftxui::vbox({result, line});
        }
    }
    return result | ftxui::center;
}

}