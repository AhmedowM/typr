#include "ContentSelector.hpp"
#include "../Theme.hpp"

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>

namespace typr::ui {

using namespace ftxui;

static std::string modeLabel(const ContentSelectorState& state) {
  int fileCount = (int)state.filePaths.size();
  int total = (int)state.builtInTypes.size() + fileCount;
  if (total == 0) return "--";

  if (state.selectedIndex >= (int)state.builtInTypes.size()) {
    int fileIdx = state.selectedIndex - (int)state.builtInTypes.size();
    if (fileIdx < fileCount) {
      return state.filePaths[fileIdx].filename().string();
    }
  }

  auto& bt = state.builtInTypes;
  if (state.selectedIndex < (int)bt.size()) {
    return std::string(ContentSource::label(bt[state.selectedIndex]));
  }
  return "--";
}

Component ContentSelector(std::shared_ptr<ContentSelectorState> state) {
  auto renderer = Renderer([state] {
    auto left = text("[F] Add a file...") | dim;
    auto middle = text(modeLabel(*state)) | bold | color(ACCENT_CYAN);
    return hbox({
      left | size(WIDTH, EQUAL, 22),
      separator(),
      middle | center | flex,
    }) | bgcolor(BG_CARD);
  });

  return CatchEvent(renderer, [state](Event event) {
    int total = (int)state->builtInTypes.size() + (int)state->filePaths.size();
    if (total == 0) return true;

    if (event == Event::ArrowLeft) {
      state->selectedIndex = (state->selectedIndex - 1 + total) % total;
      return true;
    }
    if (event == Event::ArrowRight || event == Event::Tab) {
      state->selectedIndex = (state->selectedIndex + 1) % total;
      return true;
    }
    return false;
  });
}

}
