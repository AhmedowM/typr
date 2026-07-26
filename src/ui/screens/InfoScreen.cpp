#include "InfoScreen.hpp"
#include "../BigText.hpp"
#include "../Theme.hpp"
#include "version.hpp"
#include <cpptypr.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

namespace typr::ui {

ftxui::Component InfoScreen(InfoScreenCallbacks callbacks) {
    using namespace ftxui;

    auto renderer = Renderer(std::function<Element(bool)>([onMain = callbacks.onMain](bool) {
        auto versionStr = TYPR_VERSION;

        auto box = [](Element header, Element body) {
            return vbox(Elements{
                header,
                separator(),
                body | flex,
            }) | borderRounded | flex;
        };

        auto typrBox = box(
            BigText::render("typr", ColorStatValue, true),
            vbox(Elements{
                text(versionStr) | center | bold,
                text("A C++23 TUI typing trainer") | dim | center | flex,
            })
        );

        auto cpptyprBox = box(
            BigText::render("cpptypr"),
            vbox(Elements{
                text("v" + std::string(cpptypr::Version()())) | center | bold,
                text("C++ wrapper for ctypr engine") | dim | center | flex,
            })
        );

        auto ctyprBox = box(
            BigText::render("ctypr"),
            vbox(Elements{
                text("v" + std::string(ctypr::Version()())) | center | bold,
                text("C typing engine with SQLite") | dim | center | flex,
            })
        );

        return vbox(Elements{
            BigText::render("about"),
            separator(),
            vbox(Elements{
                typrBox,
                cpptyprBox,
                ctyprBox,
            }) | flex,
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
