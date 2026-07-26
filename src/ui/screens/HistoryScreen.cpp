#include "HistoryScreen.hpp"
#include "../Theme.hpp"
#include "../BigText.hpp"
#include "../../storage/Storage.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace typr {
namespace ui {

ftxui::Component HistoryScreen(Storage& storage, HistoryScreenCallbacks callbacks) {
    using namespace ftxui;

    struct HistoryState {
        std::vector<cpptypr::SessionData> sessions;
        int selectedIndex = -1;
    };
    auto state = std::make_shared<HistoryState>();

    auto clampSelected = [state] {
        if (state->selectedIndex >= static_cast<int>(state->sessions.size())) {
            state->selectedIndex = state->sessions.empty() ? -1 : static_cast<int>(state->sessions.size()) - 1;
        }
    };

    auto fmtTime = [](std::chrono::milliseconds ms) -> std::string {
        auto sec = ms.count() / 1000;
        auto min = sec / 60;
        sec %= 60;
        if (min > 0) return std::to_string(min) + "m" + std::to_string(sec) + "s";
        return std::to_string(sec) + "s";
    };

    auto fmtDate = [](const std::string& iso) -> std::string {
        if (iso.size() < 16) return iso;
        return iso.substr(5, 5) + " " + iso.substr(11, 5);
    };

    auto pad = [](const std::string& s, int w) -> std::string {
        if (static_cast<int>(s.size()) >= w) return s.substr(0, static_cast<size_t>(w));
        return s + std::string(static_cast<size_t>(w - s.size()), ' ');
    };

    // Initial load
    state->sessions = storage.repo().getRecent(50);
    clampSelected();

    auto renderer = Renderer(std::function<Element(bool)>([state, fmtTime = std::move(fmtTime), fmtDate = std::move(fmtDate), pad = std::move(pad), &storage](bool) {
        state->sessions = storage.repo().getRecent(50);
        if (state->selectedIndex >= static_cast<int>(state->sessions.size())) {
            state->selectedIndex = state->sessions.empty() ? -1 : static_cast<int>(state->sessions.size()) - 1;
        }

        if (state->sessions.empty()) {
            return vbox(Elements{
                contain(vbox(Elements{
                    BigText::render("history"),
                    separator(),
                    text("  (no sessions yet)") | dim | center | flex,
                }) | flex),
                footer({"Esc: Menu"}),
            });
        }

        const int VISIBLE = 20;
        int n = static_cast<int>(state->sessions.size());
        int sel = state->selectedIndex;

        int start = std::max(0, sel - VISIBLE / 2);
        int end = std::min(n, start + VISIBLE);
        if (end - start < VISIBLE && start > 0) {
            start = std::max(0, end - VISIBLE);
        }

        Elements rows;

        auto header = text(pad("ID", 4) + "  " + pad("Date", 11) + "  " +
                           pad("Mode", 8) + "  " + pad("WPM", 6) + "  " +
                           pad("Acc", 5) + "  " + "Time")
                      | bold | color(Color::Cyan1);
        rows.push_back(header);
        rows.push_back(separator());

        for (int i = start; i < end; ++i) {
            const auto& s = state->sessions[i];
            bool selRow = (i == sel);

            std::string line = (selRow ? ">" : " ") +
                pad(std::to_string(s.id), 3) + "  " +
                pad(fmtDate(s.timestamp), 11) + "  " +
                pad(s.mode, 8) + "  " +
                pad(std::to_string(static_cast<int>(s.wpm)), 6) + "  " +
                pad(std::to_string(static_cast<int>(s.accuracy)) + "%", 5) + "  " +
                fmtTime(s.durationMs);

            auto elem = text(line) | (selRow ? bgcolor(BG_CARD) : bgcolor(Color::Default));
            rows.push_back(elem);
        }

        if (n > VISIBLE) {
            rows.push_back(text("  " + std::to_string(start + 1) + "-" +
                                std::to_string(end) + " of " + std::to_string(n) + "  ") | dim | center);
        }

        return vbox(Elements{
            contain(vbox(Elements{
                BigText::render("history"),
                separator(),
                vbox(std::move(rows)) | borderRounded | flex,
            }) | size(HEIGHT, LESS_THAN, 28) | flex),
            footer({"↑↓: Navigate", "Delete: Remove", "Esc: Menu"}),
        });
    }));

    auto onMain = callbacks.onMain;

    return renderer | CatchEvent(std::function<bool(Event)>([state, &storage, onMain](Event event) {
        int n = static_cast<int>(state->sessions.size());
        if (n == 0) {
            if (event == Event::Escape || event == Event::CtrlG) {
                if (onMain) onMain();
                return true;
            }
            return false;
        }

        if (event == Event::ArrowUp || event == Event::Character('k')) {
            if (state->selectedIndex > 0) {
                --state->selectedIndex;
                return true;
            }
            return false;
        }
        if (event == Event::ArrowDown || event == Event::Character('j')) {
            if (state->selectedIndex < n - 1) {
                ++state->selectedIndex;
                return true;
            }
            return false;
        }
        if (event == Event::Escape || event == Event::CtrlG) {
            if (onMain) onMain();
            return true;
        }
        if (event == Event::Delete || event == Event::Backspace || event == Event::Character('d')) {
            if (state->selectedIndex >= 0 && state->selectedIndex < n) {
                auto id = state->sessions[state->selectedIndex].id;
                [[maybe_unused]] auto deleted = storage.repo().deleteSession(id);
                state->sessions = storage.repo().getRecent(50);
                if (state->selectedIndex >= static_cast<int>(state->sessions.size())) {
                    state->selectedIndex = state->sessions.empty() ? -1 : static_cast<int>(state->sessions.size()) - 1;
                }
            }
            return true;
        }
        return false;
    }));
}

}
}
