#pragma once

#include <ftxui/component/component.hpp>
#include <functional>

namespace typr::ui {

struct InfoScreenCallbacks {
    std::function<void()> onMain;
};

ftxui::Component InfoScreen(InfoScreenCallbacks callbacks);

}
