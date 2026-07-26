#pragma once

#include <ftxui/component/component.hpp>
#include <functional>
#include <memory>

namespace typr {
class EngineBridge;
class Storage;
}

namespace typr::ui {

struct ResultsScreenCallbacks {
    std::function<void()> onRestart;
    std::function<void()> onNext;
    std::function<void()> onMain;
};

ftxui::Component ResultsScreen(EngineBridge& engine, Storage& storage, ResultsScreenCallbacks callbacks);

}
