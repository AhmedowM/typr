#include "TypingScreen.hpp"
#include "../Theme.hpp"
#include "../components/StatBox.hpp"
#include "../components/SettingsBar.hpp"
#include "../../engine/EngineBridge.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>
#include <string>

namespace typr::ui {

ftxui::Component TypingScreen(EngineBridge& engine, std::shared_ptr<TypingScreenState> state) {
    using namespace ftxui;

    EngineBridge* enginePtr = &engine;

    auto renderer = Renderer(std::function<Element(bool)>([enginePtr, state](bool) {
        auto snap = enginePtr->getSnapshot();
        bool idle = (snap.state() == cpptypr::EngineState::Idle &&
                     snap.stopCause() == cpptypr::StopCause::None);

        Element textArea;
        if (idle) {
            textArea = text("Press [SPACE] or [ENTER] to start") | center | dim | flex;
        } else {
            auto textView = snap.text();
            auto cursor = snap.cursorIndex();
            auto len = snap.length();

            Elements chars;
            for (size_t i = 0; i < len; ++i) {
                Element ch = text(std::string(1, textView[i]));
                if (i < cursor) {
                    if (snap.isIncorrect(i))
                        ch |= color(Color::Red) | bold;
                    else
                        ch |= color(Color::Green);
                } else if (i == cursor) {
                    ch |= bold | bgcolor(Color::White) | color(Color::Black);
                } else {
                    ch |= dim;
                }
                chars.push_back(ch);
            }

            textArea = hflow(chars) | flex;

            if (state->errorFlashActive.load()) {
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - state->errorFlashStart).count();
                if (elapsed < 100) {
                    textArea |= bgcolor(Color::Red);
                } else {
                    state->errorFlashActive.store(false);
                }
            }
        }

        SettingsBarConfig cfg;
        cfg.modeLabel = state->modeLabel;
        cfg.timeoutLabel = state->timeoutLabel;
        cfg.showHints = state->showSettingsHints;
        auto settingsElem = SettingsBarElement(cfg);

        auto stats = snap.stats();
        auto elapsedMs = stats.durationMs.count();
        auto elapsedSec = elapsedMs / 1000;
        auto elapsedMin = elapsedSec / 60;
        elapsedSec %= 60;
        std::string elapsedStr = elapsedMin > 0
            ? std::to_string(elapsedMin) + "m" + std::to_string(elapsedSec) + "s"
            : std::to_string(elapsedSec) + "s";
        std::string accuracyStr = std::to_string(static_cast<int>(stats.accuracy)) + "%";
        std::string speedStr = std::to_string(static_cast<int>(stats.wpm));
        std::string rawStr = std::to_string(static_cast<int>(stats.wpmRaw));

        auto statsRow = hbox(Elements{
            StatBoxElement("TIME", elapsedStr) | flex,
            StatBoxElement("ACCURACY", accuracyStr) | flex,
            StatBoxElement("SPEED", speedStr) | flex,
            StatBoxElement("RAW SPD", rawStr) | flex,
        });

        return vbox(Elements{
            settingsElem,
            textArea | borderRounded | flex,
            statsRow,
            footer({"Esc: Abort", "Ctrl+S: Mode", "Ctrl+T: Timeout", "Ctrl+G: Menu"}),
        });
    }));

    return renderer | CatchEvent(std::function<bool(Event)>([enginePtr, state](Event event) {
        auto snap = enginePtr->getSnapshot();
        bool idle = (snap.state() == cpptypr::EngineState::Idle &&
                     snap.stopCause() == cpptypr::StopCause::None);

        if (event == Event::Escape) {
            if (!idle) enginePtr->stop();
            if (state->navigateToMain) state->navigateToMain();
            return true;
        }

        if (event == Event::CtrlG) {
            if (!idle) enginePtr->stop();
            if (state->navigateToMain) state->navigateToMain();
            return true;
        }

        if (idle) {
            if (event == Event::Character(' ') || event == Event::Return) {
                enginePtr->start();
                state->showIdleHint = false;
                return true;
            }
            if (event == Event::CtrlS) {
                auto mode = enginePtr->engine().mode();
                if (mode == cpptypr::EngineMode::Strict) {
                    enginePtr->engine().setMode(cpptypr::EngineMode::Flow);
                    state->modeLabel = "FLOW";
                } else {
                    enginePtr->engine().setMode(cpptypr::EngineMode::Strict);
                    state->modeLabel = "STRICT";
                }
                return true;
            }
            if (event == Event::CtrlT) {
                auto t = enginePtr->engine().timeout();
                if (t == 0) { enginePtr->engine().setTimeout(15); state->timeoutLabel = "15s"; }
                else if (t == 15) { enginePtr->engine().setTimeout(30); state->timeoutLabel = "30s"; }
                else if (t == 30) { enginePtr->engine().setTimeout(60); state->timeoutLabel = "60s"; }
                else { enginePtr->engine().setTimeout(0); state->timeoutLabel = "none"; }
                return true;
            }
            return false;
        }

        if (event == Event::Backspace) {
            enginePtr->backspacePress();
            return true;
        }

        if (event.is_character()) {
            auto ch = event.character();
            if (!ch.empty()) {
                enginePtr->keyPress(ch[0]);
            }
            return true;
        }

        return false;
    }));
}

}
