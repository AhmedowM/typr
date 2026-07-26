#pragma once

#include <ftxui/component/component.hpp>
#include <functional>

namespace typr {
class Storage;
}

namespace typr::ui {

struct HistoryScreenCallbacks {
    std::function<void()> onMain;
};

ftxui::Component HistoryScreen(Storage& storage, HistoryScreenCallbacks callbacks);

}
