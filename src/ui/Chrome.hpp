#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace typr::ui {

ftxui::Element Header(const ftxui::Element& content = ftxui::emptyElement());
ftxui::Element Footer(const ftxui::Element& hints = ftxui::emptyElement());

}