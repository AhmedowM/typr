#include "ContentSelector.hpp"
#include "../Theme.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

namespace typr::ui {

using namespace ftxui;

static std::string modeLabel(const ContentSelectorState &state) {
    int fileCount = (int)state.filePaths.size();
    int total = (int)state.builtinTypes.size() + fileCount;
    if (total == 0) return "--";

    if (state.selectedIndex >= (int)state.builtinTypes.size()) {
        int fileIdx = state.selectedIndex - (int)state.builtinTypes.size();
        if (fileIdx < fileCount) {
            return state.filePaths[fileIdx].filename().string();
        }
    }

    auto &bt = state.builtinTypes;
    if (state.selectedIndex < (int)bt.size()) {
        return std::string(ContentSource::label(bt[state.selectedIndex]));
    }
    return "--";
}

static std::string statusText(const ContentSelectorState &state) {
    switch (state.downloadState) {
        case DownloadState::Idle:
            return "databases ready";
        case DownloadState::Downloading:
            return "downloading " + std::to_string(state.downloadPercent) + "%";
        case DownloadState::Ready:
            return "databases ready";
        case DownloadState::Failed:
            return "error downloading";
    }
    return "";
}

Component ContentSelector(std::shared_ptr<ContentSelectorState> state,
                        ContentSelectorCallbacks callbacks) {
    auto fwd = state;
    auto onAddFile = std::move(callbacks.onAddFile);

    auto inner = Renderer([fwd] {
        auto left = text(" [F] select file ") | dim;

        auto middle = hbox({
        text("<") | color(DIM_GRAY_2),
        text(" " + modeLabel(*fwd) + " ") | bold | color(ACCENT_CYAN),
        text(">") | color(DIM_GRAY_2),
        });

        Element right;
        auto st = statusText(*fwd);
        if (fwd->downloadState == DownloadState::Failed) {
            right = text(" " + st + " ") | color(ERROR_RED);
        } else if (fwd->downloadState == DownloadState::Downloading) {
            right = text(" " + st + " ") | color(PB_ORANGE);
        } else {
            right = text(" " + st + " ") | dim | color(DIM_GRAY_1);
        }

        return hbox({
            left,
            middle | center | flex,
            right,
        }) | bgcolor(BG_CARD);
    });

    auto focusable = Renderer([inner](bool focused) {
        auto elem = inner->Render();
        if (focused) {
            elem = elem | bold;
        }
        return elem;
    });

    return CatchEvent(focusable, [fwd, onAddFile](Event event) {
        int total = (int)fwd->builtinTypes.size() + (int)fwd->filePaths.size();

        if (event == Event::ArrowLeft || event == Event::Character('<')) {
            if (total > 0) {
                int cur = fwd->selectedIndex.load();
                fwd->selectedIndex.store((cur - 1 + total) % total);
            }
            return true;
        }
        if (event == Event::ArrowRight || event == Event::Character('>')) {
            if (total > 0) {
                int cur = fwd->selectedIndex.load();
                fwd->selectedIndex.store((cur + 1) % total);
            }
            return true;
        }
        if (event == Event::Character('f') && onAddFile) {
            onAddFile();
            return true;
        }
        return false;
    });
}

} // namespace typr::ui