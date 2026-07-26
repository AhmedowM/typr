#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include "../Theme.hpp"
#include <string>

namespace typr::ui {

ftxui::Component StatBox(const std::string& label, const std::string& value, ftxui::Color valueColor = typr::ui::ColorStatValue);

ftxui::Element StatBoxElement(const std::string& label, const std::string& value, ftxui::Color valueColor = typr::ui::ColorStatValue);

}