#pragma once

#include "../../app/ContentLocator.hpp"
#include "../../engine/ContentSource.hpp"

#include <ftxui/component/component.hpp>
#include <memory>
#include <string>
#include <vector>

namespace typr::ui {

struct ContentSelectorState {
  int selectedIndex = 0;
  std::vector<ContentType> builtInTypes = {
    ContentType::Sentences,
    ContentType::CommonWords,
    ContentType::RandomWords
  };
  std::vector<std::filesystem::path> filePaths;
};

ftxui::Component ContentSelector(
    std::shared_ptr<ContentSelectorState> state);

}
