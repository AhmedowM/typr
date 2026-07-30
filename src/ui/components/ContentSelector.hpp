#pragma once

#include "../../app/ContentLocator.hpp"
#include "../../engine/ContentSource.hpp"

#include <ftxui/component/component.hpp>
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace typr::ui {

struct ContentSelectorCallbacks {
    std::function<void()> onAddFile;
};

struct ContentSelectorState {
    std::atomic<int> selectedIndex{0};
    std::vector<ContentType> builtinTypes = {
        ContentType::Sentences,
        ContentType::CommonWords,
        ContentType::RandomWords
    };
    std::vector<std::filesystem::path> filePaths;
    DownloadState downloadState = DownloadState::Idle;
    int downloadPercent = 0;
};

ftxui::Component ContentSelector(
    std::shared_ptr<ContentSelectorState> state,
    ContentSelectorCallbacks callbacks);

}
