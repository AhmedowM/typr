#include "StatsScreen.hpp"
#include "../Theme.hpp"
#include "../BigText.hpp"
#include "../components/StatBox.hpp"
#include "../../storage/Storage.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>
#include <string>

namespace typr {
namespace ui {

ftxui::Component StatsScreen(Storage& storage, StatsScreenCallbacks callbacks) {
    using namespace ftxui;

    auto renderer = Renderer(std::function<Element(bool)>([&storage](bool) {
        auto total = storage.repo().count();
        auto avgWpm = storage.repo().averageWpm();
        auto best = storage.repo().bestWpm();

        std::string totalStr = std::to_string(total);
        std::string avgStr = total > 0
            ? std::to_string(static_cast<int>(avgWpm))
            : "\u2014";
        std::string bestStr = best.has_value()
            ? std::to_string(static_cast<int>(best->wpm))
            : "\u2014";

        auto graphPlaceholder = vbox(Elements{
            filler(),
            text("Graph Visualization Coming Soon") | dim | center,
            filler(),
        }) | borderRounded | flex;

        return vbox(Elements{
            BigText::render("statistics", ColorStatValue, true),
            separator(),
            hbox(Elements{
                StatBoxElement("Total Sessions", totalStr) | flex,
                StatBoxElement("Avg WPM", avgStr) | flex,
                StatBoxElement("Best WPM", bestStr) | flex,
            }),
            graphPlaceholder,
            footer({"Esc: Menu"}),
        }) | center;
    }));

    return renderer | CatchEvent(std::function<bool(Event)>([onMain = callbacks.onMain](Event event) {
        if (event == Event::Escape || event == Event::CtrlG) {
            if (onMain) onMain();
            return true;
        }
        return false;
    }));
}

}
}
