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
            })
        );
        
        auto typrInfoBox = vbox(
            Elements{
                text("A C++23 TUI typing trainer") | dim | center | flex,
            }
        ) | borderRounded;

        auto cpptyprBox = box(
            BigText::render("cpptypr"),
            vbox(Elements{
                text("v" + std::string(cpptypr::Version()())) | center | bold,
                filler(),
                text("C++ wrapper for ctypr engine") | dim | center | flex,
                filler()
            })
        );

        auto ctyprBox = box(
            BigText::render("ctypr"),
            vbox(Elements{
                text("v" + std::string(ctypr::Version()())) | center | bold,
                filler(),
                text("C typing engine with SQLite") | dim | center | flex,
                filler()
            })
        );

        return vbox(Elements{
            contain(vbox(Elements{
                BigText::render("about"),
                separator(),
                gridbox({
                    { typrBox, typrInfoBox },
                    { separator(), separator() },
                    { cpptyprBox, ctyprBox },
                }) | size(HEIGHT, LESS_THAN, 20) | flex,
            }) | flex),
            footer({"Esc: Menu"}),
        });
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
