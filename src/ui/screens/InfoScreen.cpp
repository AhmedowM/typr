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
        auto typrVer = TYPR_VERSION;
        auto cpptyprVer = std::string(cpptypr::Version()());
        auto ctyprVer = std::string(ctypr::Version()());
        auto ftxuiVer = FTXUI_VERSION;
        auto sqliteVer = SQLITE_VERSION;

        auto typrBox = vbox(Elements{
            BigText::render("typr", ACCENT_CYAN, true),
            separator(),
            text(typrVer) | center | bold,
            text("Terminal typing trainer with real-time WPM, accuracy tracking & session history") | center | dim,
        }) | borderRounded;

        auto basedOn = hbox(Elements{
            separator() | flex,
            text(" based on ") | color(ACCENT_CYAN) | center,
            separator() | flex,
        });

        auto cpptyprBox = vbox(Elements{
            BigText::render("cpptypr", Color::Default, true),
            separator(),
            text("v" + cpptyprVer) | center | bold,
            text("C++ wrapper around ctypr for building typing-test TUI components") | center | dim,
        }) | borderRounded;

        auto libEntry = [](std::string name, std::string ver, std::string desc) {
            return hbox(Elements{
                text("  " + name) | bold | size(WIDTH, EQUAL, 10),
                separator(),
                text("  " + ver) | size(WIDTH, EQUAL, 10),
                separator(),
                text("  " + desc) | dim | flex,
            });
        };

        auto libraryBox = vbox(Elements{
            libEntry("ctypr", "v" + ctyprVer, "C typing engine with per-character accuracy & SQLite-backed persistence"),
            separator(),
            libEntry("ftxui", ftxuiVer, "C++ Terminal UI framework powering the component system"),
            separator(),
            libEntry("SQLite", sqliteVer, "Self-contained SQL database engine for persistent session storage"),
        }) | borderRounded;

        return vbox(Elements{
            contain(vbox(Elements{
                BigText::render("about", Color(ACCENT_CYAN)),
                separator(),
                typrBox,
                basedOn,
                cpptyprBox,
                libraryBox | size(HEIGHT, LESS_THAN, 7),
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
