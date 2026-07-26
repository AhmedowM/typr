#pragma once

#include <ftxui/screen/color.hpp>
#include <ftxui/dom/elements.hpp>

namespace typr::ui {

inline const ftxui::Color BG_DARK       = ftxui::Color::RGB(0x12, 0x12, 0x12); // #121212
inline const ftxui::Color BG_CARD       = ftxui::Color::RGB(0x1e, 0x1e, 0x1e); // #1e1e1e
inline const ftxui::Color BORDER_MED    = ftxui::Color::RGB(0x33, 0x33, 0x33); // #333333
inline const ftxui::Color BORDER_LIGHT  = ftxui::Color::RGB(0x44, 0x44, 0x44); // #444444
inline const ftxui::Color TEXT_PRIMARY  = ftxui::Color::RGB(0xe0, 0xe0, 0xe0); // #e0e0e0
inline const ftxui::Color ACCENT_CYAN   = ftxui::Color::RGB(0x00, 0xd7, 0xff); // #00d7ff
inline const ftxui::Color STAT_GREEN    = ftxui::Color::RGB(0x00, 0xff, 0x00); // #00ff00
inline const ftxui::Color PB_ORANGE     = ftxui::Color::RGB(0xff, 0xaa, 0x00); // #ffaa00
inline const ftxui::Color DIM_GRAY_1    = ftxui::Color::RGB(0x55, 0x55, 0x55); // #555555
inline const ftxui::Color DIM_GRAY_2    = ftxui::Color::RGB(0x88, 0x88, 0x88); // #888888
inline const ftxui::Color ERROR_RED     = ftxui::Color::RGB(0xff, 0x00, 0x00); // #ff0000
inline const ftxui::Color SUCCESS_GREEN = ftxui::Color::RGB(0x00, 0xff, 0x00); // #00ff00
inline const ftxui::Color TIMEOUT_RED   = ftxui::Color::RGB(0xff, 0x00, 0x00); // #ff0000
inline const ftxui::Color FOOTER_TEXT   = ftxui::Color::RGB(0x55, 0x55, 0x55); // #555555

// Aliases used throughout UI
inline const ftxui::Color ColorLabel        = DIM_GRAY_1;
inline const ftxui::Color ColorLabelDim     = DIM_GRAY_2;
inline const ftxui::Color ColorBorder       = BORDER_MED;
inline const ftxui::Color ColorCardBackground = BG_CARD;
inline const ftxui::Color ColorStatValue    = STAT_GREEN;

constexpr int MAX_CONTENT_WIDTH = 100;

inline ftxui::Element contain(ftxui::Element e) {
    return e | ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, MAX_CONTENT_WIDTH) | ftxui::center | ftxui::flex;
}

ftxui::Element header();
ftxui::Element footer(const std::vector<std::string>& hints = {});

}