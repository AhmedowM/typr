#pragma once

#include <ftxui/component/component.hpp>
#include <functional>

namespace typr {
class Storage;
}

namespace typr::ui {

struct StatsScreenCallbacks {
    std::function<void()> onMain;
};

ftxui::Component StatsScreen(Storage& storage, StatsScreenCallbacks callbacks);

}
