#include "MainScreen.hpp"
#include "../BigText.hpp"
#include "../Theme.hpp"
#include "version.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>

namespace typr::ui {

ftxui::Component MainScreen(const MainScreenCallbacks& callbacks) {
    using namespace ftxui;

    auto onStart = callbacks.onStartPractice;
    auto onHistory = callbacks.onHistory;
    auto onStats = callbacks.onStats;

    auto flatButton = ButtonOption();
    flatButton.transform = [](const EntryState& s) {
        auto element = text(s.label) | center;
        if (s.focused) element |= inverted;
        return element;
    };

    auto startButton = Button("  START PRACTICE  ", [onStart] {
        if (onStart) onStart();
    }, flatButton);
    startButton |= size(WIDTH, EQUAL, 33) | size(HEIGHT, EQUAL, 3);

    auto historyButton = Button("  History  ", [onHistory] {
        if (onHistory) onHistory();
    }, flatButton);
    historyButton |= size(WIDTH, EQUAL, 15);

    auto statsButton = Button("  Stats  ", [onStats] {
        if (onStats) onStats();
    }, flatButton);
    statsButton |= size(WIDTH, EQUAL, 15);

    auto buttonsRow = Container::Horizontal({
        historyButton,
        statsButton,
    });

    auto container = Container::Vertical({
        startButton,
        buttonsRow,
    });

    auto renderer = Renderer(container, std::function<Element()>([startButton, historyButton, statsButton] {
        auto body = vbox(Elements{
            filler(),
            BigText::render("TYPR", Color::Cyan1, true),
            text("version " TYPR_VERSION) | dim | italic | center,
            filler(),
            startButton->Render() | borderRounded | center,
            filler(),
            hbox(Elements{historyButton->Render(), statsButton->Render()}) | center,
            filler(),
        }) | center;

        return vbox(Elements{
            body | flex,
            footer({"Q: Quit", "↑↓: Navigate", "Enter: Select"}),
        });
    }));

    auto catchHandler = std::function<bool(Event)>([onQuit = callbacks.onQuit](Event event) {
        if (event == Event::Character('q') || event == Event::Escape) {
            if (onQuit) onQuit();
            return true;
        }
        return false;
    });
    return renderer | CatchEvent(std::move(catchHandler));
}

}