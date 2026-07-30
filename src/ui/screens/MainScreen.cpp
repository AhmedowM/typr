#include "MainScreen.hpp"
#include "../BigText.hpp"
#include "../Theme.hpp"
#include "version.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>

namespace typr::ui {

ftxui::Component MainScreen(const MainScreenCallbacks& callbacks,
                            std::shared_ptr<ContentSelectorState> contentState) {
    using namespace ftxui;

    auto onStart  = callbacks.onStartPractice;
    auto onHistory = callbacks.onHistory;
    auto onStats  = callbacks.onStats;
    auto onInfo   = callbacks.onInfo;
    auto onQuit   = callbacks.onQuit;

    ContentSelectorCallbacks csCB;
    csCB.onAddFile = [onStart] {
        // Placeholder: file-open dialog will be added later
    };
    auto selector = ContentSelector(contentState, csCB);

    auto startOpt = ButtonOption();
    startOpt.transform = [](const EntryState& s) {
        auto prefix = text(s.focused ? "> " : "  ");
        auto element = text(s.label) | center;
        if (s.focused) {
            prefix |= bgcolor(BG_CARD);
            element |= bgcolor(BG_CARD);
        }
        return hbox(Elements{prefix, element | flex}) | flex;
    };
    auto startButton = Button("  START PRACTICE  ", [onStart] {
        if (onStart) onStart();
    }, startOpt);
    startButton |= size(WIDTH, EQUAL, 33) | size(HEIGHT, EQUAL, 1);

    auto navButtonOption = ButtonOption();
    navButtonOption.transform = [](const EntryState& s) {
        auto prefix = text(s.focused ? "> " : "  ");
        auto label = text(s.label) | center | flex;
        if (s.focused) {
            prefix |= bgcolor(BG_CARD);
            label |= bgcolor(BG_CARD);
        }
        return hbox(Elements{prefix, label}) | flex;
    };

    auto historyButton = Button("History", [onHistory] {
        if (onHistory) onHistory();
    }, navButtonOption);

    auto statsButton = Button("Stats", [onStats] {
        if (onStats) onStats();
    }, navButtonOption);

    auto infoButton = Button("Info", [onInfo] {
        if (onInfo) onInfo();
    }, navButtonOption);

    auto quitButton = Button("Exit", [onQuit] {
        if (onQuit) onQuit();
    }, navButtonOption);

    auto navButtons = Container::Vertical({
        historyButton,
        statsButton,
        infoButton,
        quitButton,
    });

    auto container = Container::Vertical({
        startButton,
        selector,
        navButtons,
    });

    auto renderer = Renderer(container, std::function<Element()>([selector, startButton, historyButton, statsButton, infoButton, quitButton] {
        return vbox(Elements{
            contain(vbox(Elements{
                filler(),
                BigText::render("TYPR", ACCENT_CYAN, true),
                text("version " TYPR_VERSION) | dim | italic | center,
                filler(),
                startButton->Render() | borderRounded | color(STAT_GREEN) | center,
                separator(),
                selector->Render() | center,
                separator(),
                vbox(Elements{
                    historyButton->Render() | color(ACCENT_CYAN),
                    statsButton->Render() | color(ACCENT_CYAN),
                    infoButton->Render() | color(ACCENT_CYAN),
                    quitButton->Render() | color(ACCENT_CYAN),
                }) | borderRounded | color(ACCENT_CYAN),
                filler(),
            }) | flex),
            footer({"Q: Quit", "↑↓: Navigate", "Enter: Select", "←→: Cycle Mode", "F: Add File"}),
        });
    }));

    auto catchHandler = std::function<bool(Event)>([onInfo, onQuit](Event event) {
        if (event == Event::Character('q') || event == Event::Escape) {
            if (onQuit) onQuit();
            return true;
        }
        if (event == Event::Character('i')) {
            if (onInfo) onInfo();
            return true;
        }
        return false;
    });
    return renderer | CatchEvent(std::move(catchHandler));
}

}