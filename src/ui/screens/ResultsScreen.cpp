#include "ResultsScreen.hpp"
#include "../BigText.hpp"
#include "../Theme.hpp"
#include "../components/StatBox.hpp"
#include "../../engine/EngineBridge.hpp"
#include "../../storage/Storage.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>
#include <string>
#include <vector>

namespace typr::ui {

ftxui::Component ResultsScreen(EngineBridge& engine, Storage& storage, ResultsScreenCallbacks callbacks) {
    using namespace ftxui;
    using namespace cpptypr;

    EngineBridge* enginePtr = &engine;
    Storage* storagePtr = &storage;
    auto onRestart = callbacks.onRestart;
    auto onNext = callbacks.onNext;
    auto onMain = callbacks.onMain;

    auto isNewBest = std::make_shared<bool>(false);
    {
        auto stats = engine.stats();
        auto best = storage.repo().bestWpm();
        *isNewBest = !best.has_value() || stats.wpm > best->wpm;
    }

    auto renderer = Renderer(std::function<Element(bool)>([enginePtr, storagePtr, isNewBest](bool) {
        auto snap = enginePtr->getSnapshot();
        auto stats = snap.stats();
        bool completed = (snap.stopCause() == StopCause::Finished);

        auto headerElem = BigText::render(
            completed ? "completed" : "timeout",
            completed ? SUCCESS_GREEN : TIMEOUT_RED,
            true
        );

        Elements leftCol;
        if (*isNewBest) {
            leftCol.push_back(text("  NEW PERSONAL BEST!  ") | color(PB_ORANGE) | bold | center);
        }

        auto elapsedMs = stats.durationMs.count();
        auto elapsedSec = elapsedMs / 1000;
        auto elapsedMin = elapsedSec / 60;
        elapsedSec %= 60;
        std::string timeStr = elapsedMin > 0
            ? std::to_string(elapsedMin) + "m" + std::to_string(elapsedSec) + "s"
            : std::to_string(elapsedSec) + "s";

        auto statsGrid = gridbox({
            { StatBoxElement("WPM", std::to_string(static_cast<int>(stats.wpm))), 
            StatBoxElement("ACCURACY", std::to_string(static_cast<int>(stats.accuracy)) + "%") },
            { StatBoxElement("TIME", timeStr), 
            StatBoxElement("RAW WPM", std::to_string(static_cast<int>(stats.wpmRaw))) },
        });

        leftCol.push_back(statsGrid | yflex);

        auto leftPanel = vbox(std::move(leftCol)) | size(WIDTH, EQUAL, 50);

        auto recent = storagePtr->repo().getRecent(5);
        Elements historyItems;
        for (const auto& s : recent) {
            auto sec = s.durationMs.count() / 1000;
            auto min = sec / 60;
            sec %= 60;
            std::string line = s.mode + "  " +
                std::to_string(static_cast<int>(s.wpm)) + " WPM  " +
                std::to_string(static_cast<int>(s.accuracy)) + "%  " +
                (min > 0 ? std::to_string(min) + "m" : "") +
                std::to_string(sec) + "s";
            historyItems.push_back(text(line));
        }
        if (historyItems.empty()) {
            historyItems.push_back(text("  (no sessions yet)") | dim | center);
        }

        auto rightPanel = vbox(Elements{
            text("Recent History") | color(PB_ORANGE) | bold | center,
            separator(),
            vbox(std::move(historyItems)) | flex,
        }) | borderRounded;

        return vbox(Elements{
            contain(vbox(Elements{
                headerElem,
                separator(),
                hbox(Elements{
                    leftPanel | flex_grow_factor(2),
                    rightPanel | flex_grow_factor(1),
                }) | size(HEIGHT, LESS_THAN, 10) | flex,
                }) | flex),
            footer({"Ctrl+R: Restart", "Ctrl+N: Next", "Esc: Menu"}),
        });
    }));

    return renderer | CatchEvent(std::function<bool(Event)>([onRestart, onNext, onMain](Event event) {
        if (event == Event::Escape || event == Event::CtrlG) {
            if (onMain) onMain();
            return true;
        }
        if (event == Event::CtrlR) {
            if (onRestart) onRestart();
            return true;
        }
        if (event == Event::CtrlN) {
            if (onNext) onNext();
            return true;
        }
        return false;
    }));
}

}
