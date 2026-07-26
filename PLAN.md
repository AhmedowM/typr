# Typr-TUI — Build Plan

A C++23 TUI + CLI typing tool built with **FTXUI** on top of **cpptypr** (C++ wrapper) and **ctypr** (C engine). This plan tracks work across sessions; check off items as they complete.

Reference: see `tui.md` for the framework-agnostic TUI spec. This document defines the C++/FTXUI implementation strategy, project structure, phasing, and decisions.

---

## 1. Confirmed Decisions

| Area | Decision |
|---|---|
| Language standard | C++23 (matches cpptypr) |
| Build system | CMake ≥ 3.25, FetchContent for all deps |
| Engine library | `cpptypr` (FetchContent → `https://github.com/AhmedowM/cpptypr.git`) — pulls in `ctypr` transitively |
| TUI library | `FTXUI` (FetchContent → `https://github.com/ArthurSonzogni/FTXUI.git`, default → v7.0.1 as of Jul 2026) |
| TLS workaround | Configure CMake with `-DCMAKE_TLS_VERIFY=0` (workstation constraint) |
| Per-char render state | **Owner of cpptypr/ctypr will upstream a `Snapshot` struct** exposing `text`, `cursor`, `incorrectIndices` from the engine. TUI blocks on this addition until a release with the API is available; until then the TUI code is written against the agreed interface and tested with a local stub. |
| Event model | Use cpptypr callbacks (`onStarted`, `onTimeout`, `onFinished`, `onIncorrectKeystroke`) to drive TUI state transitions. The 100ms timer described in tui.md §4.2 is still used for live stat updates and rendering, but state transitions fire immediately via callbacks. |
| Content source (v1) | `ContentProvider::fromString` backed by bundled English sentences embedded as a `static constexpr` string or shipped `assets/sentences.txt`. |
| Persistence | `cpptypr::Repository` pointing at `<user_data_dir>/typr/sessions.db` (`%APPDATA%/typr/sessions.db` on Windows, `~/.local/share/typr/sessions.db` on Linux). Auto-save enabled. |
| Phasing | Screen-by-screen vertical slices (Main → Typing → Results → History → Stats). Each screen ends in a runnable binary demo-able before the next begins. |

---

## 2. cpptypr Snapshot API (as of v0.4.1)

The Snapshot API was upstreamed by the library owner. The real API differs from the earlier tentative plan:

```cpp
// In <cpptypr/snapshot.hpp> (included via engine.hpp)

namespace cpptypr {

enum class EngineState { Idle, Running, Paused, Error };
enum class StopCause { None, Timeout, Finished, User, Error, Unknown };

[[nodiscard]] std::string_view toString(EngineState) noexcept;
[[nodiscard]] std::string_view toString(StopCause) noexcept;

class Snapshot {
public:
    [[nodiscard]] std::string_view text() const noexcept;           // current content
    [[nodiscard]] size_t length() const noexcept;                   // content length
    [[nodiscard]] uint32_t cursorIndex() const noexcept;            // cursor position
    [[nodiscard]] char expectedChar() const noexcept;               // char at cursor
    [[nodiscard]] bool isIncorrect(size_t index) const;            // was key at index wrong?
    [[nodiscard]] SessionStats stats() const noexcept;              // WPM, accuracy, etc.
    [[nodiscard]] EngineState state() const noexcept;               // Idle/Running/...
    [[nodiscard]] StopCause stopCause() const noexcept;             // why it stopped
};

// In engine.hpp:
class Engine {
    [[nodiscard]] Snapshot getSnapshot();  // note: non-const, returns Snapshot by value
    // ...
};

} // namespace cpptypr
```

**Key differences from the original plan:**
- `getSnapshot()` (not `snapshot()`)
- `Snapshot::isIncorrect(size_t index)` (not `incorrectIndices()`)
- `cursorIndex()` returns `uint32_t` (not `size_t`), `length()` returns `size_t`
- `SessionStats::accuracy` is percentage 0.0–100.0 (not 0.0–1.0 ratio)
- `EngineState` and `StopCause` enums live in `snapshot.hpp`, not as separate engine queries
- `Snapshot` is a class (heap-allocated via `unique_ptr<::EngineSnapshot>`), returned by value — cheap move

---

## 3. Project Structure

```
typr-tui/
├── CMakeLists.txt                 # Top-level: project, C++23, FetchContent, subdirs
├── cmake/
│   └── Dependencies.cmake         # FetchContent for FTXUI + cpptypr, TLS_VERIFY handling
├── assets/
│   └── sentences.txt              # Bundled practice content (v1 source)
├── src/
│   ├── main.cpp                   # Entry: parse CLI args, bootstrap AppController, run FTXUI loop
│   ├── app/
│   │   ├── AppController.hpp      # Owns engine, repository, content provider, screen stack
│   │   ├── AppController.cpp      # Screen routing, action callbacks (cycle_mode, restart, etc.)
│   │   ├── SnapshotView.hpp       # Thin adapter around cpptypr::EngineSnapshot
│   │   └── Paths.hpp              # Cross-platform user_data_dir resolution
│   ├── ui/
│   │   ├── Theme.hpp              # Color palette (tui.md §1), FTXUI Color helpers
│   │   ├── Theme.cpp
│   │   ├── Chrome.hpp             # Header + Footer components, dimensions
│   │   ├── Chrome.cpp
│   │   ├── BigText.hpp            # ASCII block-letter renderer (tui.md §5)
│   │   ├── BigText.cpp            # 4-row glyph table + renderBigText()
│   │   ├── components/
│   │   │   ├── StatBox.hpp        # Reusable bordered stat card (value + label)
│   │   │   ├── StatBox.cpp
│   │   │   └── SettingsBar.hpp    # MODE/TIMEOUT label bar with cycling hints
│   │   └── screens/
│   │       ├── ScreenBase.hpp     # Common interface: render(), handleEvent(), onMount()
│   │       ├── MainScreen.hpp/cpp
│   │       ├── TypingScreen.hpp/cpp
│   │       ├── ResultsScreen.hpp/cpp
│   │       ├── HistoryScreen.hpp/cpp
│   │       └── StatsScreen.hpp/cpp
│   ├── engine/
│   │   ├── EngineBridge.hpp       # Wraps cpptypr::Engine for TUI consumption
│   │   ├── EngineBridge.cpp       # Keyboard event routing, snapshot polling, callback wiring
│   │   ├── ContentSource.hpp      # Loads bundled sentences, exposes ContentProvider
│   │   └── ContentSource.cpp
│   └── storage/
│       ├── Storage.hpp            # Repository bootstrap, user_data_dir setup
│       └── Storage.cpp
├── tests/                         # (optional) standalone tests for BigText, SnapshotView
├── tui.md                         # Original framework-agnostic spec (preserved)
└── PLAN.md                        # This file (mirror in repo root once edits allowed)
```

### Rationale

- `app/` owns routing + engine lifecycle — matches the AppController role from tui.md §2.
- `ui/` is pure FTXUI and holds no engine references; it receives data via render args.
- `engine/` isolates cpptypr usage so the upstream API rename touches one file.
- `storage/` keeps SQLite path resolution off the hot path.

---

## 4. CMake Design

`CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.25)
project(typr-tui LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Workaround for this workstation's TLS cert verification
if(NOT CMAKE_TLS_VERIFY)
    set(CMAKE_TLS_VERIFY OFF CACHE BOOL "" FORCE)
    set(CMAKE_TLS_VERIFY_CURL_SEND OFF CACHE BOOL "" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Dependencies)            # FetchContent FTXUI + cpptypr

add_executable(typr
    src/main.cpp
    src/app/AppController.cpp
    src/ui/Theme.cpp
    src/ui/Chrome.cpp
    src/ui/BigText.cpp
    src/ui/components/StatBox.cpp
    src/ui/screens/MainScreen.cpp
    src/ui/screens/TypingScreen.cpp
    src/ui/screens/ResultsScreen.cpp
    src/ui/screens/HistoryScreen.cpp
    src/ui/screens/StatsScreen.cpp
    src/engine/EngineBridge.cpp
    src/engine/ContentSource.cpp
    src/storage/Storage.cpp
)

target_include_directories(typr PRIVATE src)
target_link_libraries(typr PRIVATE
    FTXUI::component
    cpptypr::cpptypr   # or just `cpptypr` per cpptypr's namespace config
)
```

`cmake/Dependencies.cmake`:

```cmake
include(FetchContent)

FetchContent_Declare(
    FTXUI
    GIT_REPOSITORY https://github.com/ArthurSonzon/FTXUI.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(FTXUI)

FetchContent_Declare(
    cpptypr
    GIT_REPOSITORY https://github.com/AhmedowM/cpptypr.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(cpptypr)
```

Build invocation:

```
cmake -B build -DCMAKE_TLS_VERIFY=0
cmake --build build --config Release
./build/typr                              # default run
```

---

## 5. Layouts (FTXUI mapping for each screen)

### Common chrome

```
ftxui::Container::Vertical({
    header,             // fixed 1 row, cyan bold text on dark
    screen_body,        // flex (1fr)
    footer              // fixed 1 row, dim gray keybinding hints
})
```

`header`: `text("TYPR") | bold | color(cyan) | bgcolor(#1e1e1e) | center`
`footer`: `text("Ctrl+Q Quit | Ctrl+G Menu | …") | dim | bgcolor(#1e1e1e)`

### 5.1 MainScreen

```
vbox({
    filler,
    BigText("TYPR") | color(cyan) | bold | center,    // 4 rows
    text("version 0.7.2")        | dim | italic | center,
    filler,
    start_button | borderRounded | size(WIDTH, 33) | size(HEIGHT, 3),
    hbox({ history_button, stats_button }) | center,
    filler,
})
```

Buttons are `ftxui::Button` with `ButtonOption::Simple()` and custom animated border. Clicks emit callbacks to `AppController::navigateTo("typing" | "history" | "stats")`.

### 5.2 TypingScreen

```
vbox({
    settings_bar,                                       // rounded border, h-box: mode | timeout
    vbox({ text_area }) | borderRounded | flex,         // 1fr height
    hbox({ stat_box("TIME", elapsed),
           stat_box("ACCURACY", acc),
           stat_box("SPEED", wpm),
           stat_box("RAW SPD", raw) }),                 // 4 × 1fr
})
```

`text_area` is a custom `ftxui::Component` (or `ftxui::Element` built from `snapshot`) that:
- Splits `snapshot.text` into per-character `text(string(1, ch)) | color(...)` elements.
- Coloring: index < cursor → green; index in incorrectIndices → red; index == cursor → `bgcolor(white) | color(black)` (inverse); index > cursor → dim.
- Hard wraps at terminal width using FTXUI's `flexbox` or `vbox` of `hbox`-rows.
- Idle state shows centered `text("Press [SPACE] or [ENTER] to start")`.

`settings_bar` labels show `(Ctrl+S)` and `(Ctrl+T)` hints when idle; show active `MODE: [STRICT]` / `TIMEOUT: [30s]` when running.

### 5.3 ResultsScreen

```
vbox({
    BigText(state == Completed ? "completed" : "timeout") | color(state_color),
    hbox({
        vbox({                                       // 2fr
            personal_best_indicator,                 // hidden or orange flame + label
            hbox({ stat_box(WPM), stat_box(Accuracy) }),
            hbox({ stat_box(Time),  stat_box(RawWPM) }),
        }) | flex_grow(2),
        vbox({                                       // 1fr
            text("Recent History") | color(orange),
            history_list,                            // ftxui::Menu with cursor
        }) | borderRounded,
    }) | flex_grow(3),
    text("Press [Ctrl+R] Restart | [Ctrl+N] Next | [Ctrl+G] Menu"),
})
```

`history_list` is `ftxui::Menu` populated from `Repository::getRecent(5)` at screen mount.

### 5.4 HistoryScreen

`ftxui::Table` with columns `ID | Date | Mode | WPM | Acc | Time`. Built from `Repository::getRecent(50)`. Row cursor via `Menu`-style focus. `Delete` key calls `repository.deleteSession(id)`, refresh, and emits a small toast/notification (FTXUI has no toast primitive; we show a one-line transient message above the footer).

### 5.5 StatsScreen

Three `StatBox`es for `Total Sessions | Average WPM | Personal Best` from `Repository::count()` / `averageWpm()` / `bestWpm()`. Below them a bordered placeholder area with dim text "Graph Visualization Coming Soon".

---

## 6. Input Handling

Global keybindings (handled in `AppController::Handle` wrapping the active screen):

| Key | Action |
|---|---|
| `Ctrl+Q` / `Ctrl+C` | `screen.Exit()` — quit |
| `Ctrl+G` | `navigateTo("main")` |

Per-screen handlers:

- **TypingScreen**: Captures all printable characters via the screen-level `CatchEvent` filter. Idle phase routes only Space/Enter (start), Ctrl+S (cycle mode), Ctrl+T (cycle timeout). Active phase forwards printable + Enter to `engine.keyPress(ch)` and Backspace to `engine.backspacePress()` in flow mode only.
- **ResultsScreen**: Ctrl+R (restart same text), Ctrl+N (load next content chunk). On next failure, show inline error.
- **HistoryScreen**: Up/Down to move cursor, Delete to remove.
- **StatsScreen**: No local bindings.

### Event callback wiring (in `EngineBridge`)

```cpp
engine.onStarted   ([this]{ state = Running;               notify(); });
engine.onFinished  ([this]{ lastStats = engine.stats();
                            lastStopCause = Finished;       navigate("results"); });
engine.onTimeout   ([this]{ lastStats = engine.stats();
                            lastStopCause = Timeout;        navigate("results"); });
engine.onIncorrectKeystroke([this]{ flashError(); });      // red bg 100ms then restore
```

`navigate("results")` posts an FTXUI event via `ScreenInteractive::PostEvent` to break out of the current render loop iteration cleanly.

### 100ms tick

`std::thread` running `std::this_thread::sleep_for(100ms)` + `screen.PostEvent(tick_event)` to re-render. The tick handler refreshes stat labels and reads the latest snapshot. State transitions themselves fire from cpptypr callbacks, not from ticks.

---

## 7. Phasing / Milestones

### Phase 0 — Scaffolding (1 session) ✓ COMPLETED
- [x] Write top-level `CMakeLists.txt` and `cmake/Dependencies.cmake`.
- [x] Verify FTXUI + cpptypr fetch + link on workstation (with `-DCMAKE_TLS_VERIFY=0`).
- [x] Create `src/main.cpp` with a "hello FTXUI" window showing the engine state.
- [x] Confirm binary runs and exit. Commit baseline.

### Phase 1 — Engine Bridge + Persistence plumbing (1 session) ✓ COMPLETED
- [x] Implement `Paths::userDataDir()` (cross-platform).
- [x] Implement `Storage::bootstrap()` opening/creating `typr/sessions.db`.
- [x] Implement `ContentSource::makeProvider()` returning `cpptypr::ContentProvider::fromString` over `assets/sentences.txt`.
- [x] Implement `EngineBridge` ctor: builds `cpptypr::Engine` (default StrictMode, 30s), wires all callbacks.
- [x] Verify with a tiny CLI smoke-test in `main.cpp` that runs one full session synchronously and prints stats.

> Blocks here until upstream `Snapshot` API is in cpptypr. Meanwhile, develop against a local static copy of the assumed API and gate compile with `#ifdef TYPR_HAS_SNAPSHOT`.

### Phase 2 — MainScreen vertical slice (1 session) ✓ COMPLETED
- [x] Implement `Theme.hpp/cpp` (colors as `ftxui::Color` constants).
- [x] Implement `Chrome` header/footer.
- [x] Implement `BigText` glyph table (a-z, space, !) and `renderBigText`.
- [x] Implement `MainScreen` with logo, version, three buttons.
- [x] Wire navigation: Start → TypingScreen (stub), History → HistoryScreen (stub), Stats → StatsScreen (stub).
- [x] Verify clickable + keyboard navigation. Demos runnable: `./typr` shows main menu.

### Phase 3 — TypingScreen vertical slice (2 sessions) ✓ COMPLETED
- [x] Implement per-character `text_area` renderer from `EngineSnapshot`.
- [x] Implement `StatBox` reusable component.
- [x] Implement `SettingsBar` with mode/timeout labels + cycling hints.
- [x] Wire keyboard: idle routing + active routing per tui.md §4.
- [x] Wire 100ms tick thread + `PostEvent` integration.
- [x] Implement error flash (red bg → restore after 100ms) using `onIncorrectKeystroke`.
- [x] Implement `AppController::navigate("results")` triggered by `onFinished`/`onTimeout`.
- [x] Verify end-to-end: type a sentence → reach COMPLETED/TIMEOUT → auto-navigate.

### Phase 5 — HistoryScreen vertical slice (1 session) ✓ COMPLETED
- [x] Table-like display with columns (ID, Date, Mode, WPM, Acc, Time) populated by `getRecent(50)`.
- [x] Up/Down navigation with row highlight, Delete key with row removal.
- [x] Focusable fix: `Renderer(std::function<Element(bool)>)` instead of standalone Renderer.
- [x] Verify after running several sessions that history reflects them.

### Phase 6 — StatsScreen vertical slice (0.5 session) ✓ COMPLETED
- [x] Three StatBox summaries from `count()`, `averageWpm()`, `bestWpm()`.
- [x] BigText "statistics" header.
- [x] Graph placeholder area with dim "Graph Visualization Coming Soon" text.
- [x] Focusable fix: `Renderer(std::function<Element(bool)>)`.

### Phase 7 — Polish (1 session)
- [ ] Theme audit against tui.md §1 hex codes.
- [ ] Border style audit (rounded everywhere specified).
- [ ] Resize behavior / min terminal size.
- [ ] Error path: repository open failure, content file missing — show inline error + fallback to bundled defaults.
- [ ] README with build + CLI usage.
- [ ] Final smoke run through all 5 screens.

---

## 8. Cross-Platform Notes

- **User data dir**:
  - Windows: `%APPDATA%\typr\sessions.db`
  - Linux: `$XDG_DATA_HOME/typr/sessions.db` or `~/.local/share/typr/sessions.db`
  - macOS: `~/Library/Application Support/typr/sessions.db`
  - Mirror logic in `Paths.hpp`. Avoid `<filesystem>` quirks on missing `%APPDATA%`.
- **Terminal detection**: FTXUI handles TTY/raw mode automatically. No extra setup.
- **cpptypr CMake**: it itself does `FetchContent` on `ctypr`, which fetches SQLite — first build needs network (no SQLite preinstalled). Our `-DCMAKE_TLS_VERIFY=0` precaution applies to all recursive fetches.
- **Compiler**: confirm MSVC supports C++23 features used (we keep to safe subset: `std::span`, `std::expected`-free, designated initializers). If MSVC issues arise, fall back to C++20.

---

## 9. Risks & Mitigations

| Risk | Mitigation |
|---|---|
| Upstream `Snapshot` API shape differs from plan | `SnapshotView.hpp` is a single adapter file; changes localize to ~30 LOC. Block Phase 3 start until upstream lands; until then Phase 0–2 work can proceed (no engine text rendering needed yet). |
| SQLite FetchContent fails without network | Add a `TYPR_USE_SYSTEM_SQLITE` fallback later; v1 requires network on first build, document in README. |
| FTXUI `main` branch breaks ABI | Pin `GIT_TAG` to a verified commit once integration passes. |
| Per-char rendering slow at long text | Cap `assets/sentences.txt` content to ~600 chars per chunk via `ContentProvider::setContentLimit` and Formatter chunking (already in ctypr). |
| Timeout not checked between keystrokes | Engine only calls `checkTimeout()` inside `keyPress()`/`backspacePress()`. If user is idle, timeout never fires. **Mitigation**: either upstream an `engineTick()` to ctypr, or have the 100ms TUI timer poll `getSnapshot()` and manually fire timeout transition. Phase 3 blocked on this decision. |
| Thread safety between 100ms tick thread and FTXUI loop | Always mutate engine + UI state through `ScreenInteractive::PostEvent`; never paint from the tick thread directly. cpptypr callbacks can fire on the engine's own thread — same rule. |

---

## 10. Open Questions / Follow-ups

- [x] Snapshot API upstreamed — cpptypr v0.4.1, ctypr v1.2.0. Both pinned in `cmake/Dependencies.cmake`.
- [ ] Timeout between keystrokes: need `engineTick()` upstreamed or a workaround.
- [ ] Decide on a final stat-box label text vs tui.md ("RAW SPD" vs "RAW SPEED").
- [ ] Notification (toast) UX for HistoryScreen delete: simple one-line vs animated. Defer to Phase 5.
- [ ] Future: WPM trend graph in StatsScreen using a simple braille/ascii line chart (post-v1).
- [ ] Decide on a final stat-box label text vs tui.md ("RAW SPD" vs "RAW SPEED").
- [ ] Notification (toast) UX for HistoryScreen delete: simple one-line vs animated. Defer to Phase 5.
- [ ] Future: WPM trend graph in StatsScreen using a simple braille/ascii line chart (post-v1).

---

## 11. Session Handoff Notes

- After each session, update this file: check off completed items, log blockers under "Open Questions", and note the commit SHA reached.
- The first runnable binary is the success criterion of **Phase 0** alone.
- "Demo-able" means `./build/typr` runs and exercises the screen of the phase just completed without crashing.

**Session 1 (Jul 24, 2026):** Phase 0 completed. CMake structure built. FTXUI 7.0.1 + cpptypr v0.4.1 + ctypr v1.2.0 linked. Binary `build/typr.exe` (~4.3 MB) runs and shows engine state. Snapshot API confirmed available.

**Session 2 (Jul 24, 2026):** Phase 1 completed. `Paths.hpp`, `Storage.hpp/cpp`, `ContentSource.hpp/cpp`, `EngineBridge.hpp/cpp` created. CLI smoke test confirms: engine loads 483 characters, tracks all keystrokes (483/483 correct), session completes. SQLite DB created at `%APPDATA%/typr/sessions.db`. **Discovered**: timeout is only checked on `keyPress()`/`backspacePress()` — between keystrokes the engine won't auto-timeout. Need to address this before Phase 3 (TypingScreen). Next: **Phase 2 — MainScreen vertical slice.**

**Session 3 (Jul 24, 2026):** Phase 2 completed. `Theme.hpp/cpp`, `Chrome.hpp/cpp`, `BigText.hpp/cpp`, `MainScreen.hpp/cpp`, `AppController` created. Binary crashed on first render (exit code 3) due to **dangling references** in `[&]` lambdas within `MainScreen()` — local Component variables went out of scope after the function returned, causing a crash when the renderer tried to access them. Fixed by capturing `std::function` callbacks and `Component` shared_ptrs by value instead of by reference. Main menu now renders correctly with TYPR logo, version, three buttons (Start Practice, History, Stats).

**Session 4 (Jul 24, 2026):** Phase 3 completed. Major refactoring:
- AppController redesigned to use `Container::Tab` for screen switching (previously returned a single component per call)
- Added 100ms tick thread calling `engine.tick()` + `screen.PostEvent(Event::Custom)` for periodic re-renders and timeout detection
- `TypingScreen.hpp/cpp` created: per-character snapshot rendering (green/red/inverse/dim coloring), settings bar, stat boxes (TIME/ACCURACY/SPEED/RAW SPD), idle hint, error flash (100ms red bg on incorrect keystroke), keyboard event handling (printable → keyPress, Backspace → backspacePress, Space/Enter → start, Escape/Ctrl+G → Main, Ctrl+S/T → cycle mode/timeout)
- Engine callbacks wired for navigation: onFinished/onTimeout → pending nav → tick thread → tab switch to Results
- Navigation callbacks added to all stub screens (Results, History, Stats) for Escape/Ctrl+G → Main
- `Theme::footer()` changed to accept only screen-specific hints (no more hardcoded global shortcuts)
- MainScreen enhanced with CatchEvent for 'q' quit and per-screen footer hints

---

**Session 5 (Jul 26, 2026):** Phase 3 bug fixes:
- Fixed three bugs blocking TypingScreen usability:
  1. **Session started immediately** — removed `m_engine.start()` from START PRACTICE callback; engine now starts on SPACE/ENTER in idle state.
  2. **TypingScreen unresponsive to keyboard** — standalone `Renderer(std::function<Element()>)` returns `Focusable() = false` (no children). `TabContainer::ContainerBase::OnEvent` checks `Focused()` which requires `Focusable()`, dropping all keyboard events. Fixed by switching to `Renderer(std::function<Element(bool)>)` which overrides `Focusable() = true`.
  3. **Dangling reference risk** — `[&engine, state]` lambdas captured a function parameter reference; changed to capture `EngineBridge*` by value.
- Note: stub screens (Results/History/Stats) still have the same Focusable bug — fixed when they get real implementations in Phases 4–6.

---

## Phase 4 — ResultsScreen vertical slice (1 session) ✓ COMPLETED

- [x] Implement BigText header ("COMPLETED" / "TIMEOUT") with color-coded state.
- [x] Implement personal-best detection: query `Repository::bestWpm()` before current save; compare to current wpm.
- [x] Implement 4-stat grid (WPM, Accuracy, Time, Raw WPM) matching Phase 3's StatBox layout.
- [x] Implement Recent History side panel from `Repository::getRecent(5)`.
- [x] Implement Ctrl+R (restart same text) and Ctrl+N (next content chunk + restart).
- [x] Fix Focusable bug: switch standalone `Renderer(std::function<Element()>)` to `Renderer(std::function<Element(bool)>)` so Escape/Ctrl+G navigation works.
- [x] Wire `onFinished`/`onTimeout` → navigate to ResultsScreen with session stats.
- [x] Build fixes: `Theme::` → direct color constants (SUCCESS_GREEN, etc. are namespace-level, not in a Theme struct).
- [x] Verify end-to-end: type a session → land on ResultsScreen with correct stats → Ctrl+R restarts → Ctrl+N loads new text.

---

**Session 6 (Jul 26, 2026):** Phases 4–6 completed:
- **Phase 4 ResultsScreen:** BigText header (completed/timeout color-coded), personal-best detection via `Repository::bestWpm()`, 4-stat grid (WPM, Accuracy, Time, Raw WPM) matching Phase 3 StatBox layout, Recent History side panel from `getRecent(5)`, Ctrl+R/Ctrl+N/Escape keyboard handlers, auto-save enabled on engine.
- **Phase 5 HistoryScreen:** Session list with columnar display (ID, Date, Mode, WPM, Acc, Time), Up/Down row navigation with background highlight, Delete key removes selected session via `Repository::deleteSession()` with auto-reload, Focusable fix (bool-based Renderer), Escape→Main navigation.
- **Phase 6 StatsScreen:** BigText "statistics" header, three StatBox summaries from `count()`, `averageWpm()`, `bestWpm()`, Graph Visualization placeholder, Focusable fix, Escape→Main navigation.
- **Build fixes:** `Theme::` → direct color constants (`SUCCESS_GREEN` etc. are namespace-level in `typr::ui`, not a `Theme` struct), header namespace corrected from `typr` to `typr::ui` to match project convention.

---

## 13. Completed Upstream Work — Snapshot API

The Snapshot API was upstreamed by the library owner in:
- `ctypr` v1.2.0 — adds `engineGetSnapshot()` C API   
- `cpptypr` v0.4.1 — C++ `Snapshot` class with `Engine::getSnapshot()`  
- Both are pinned in `cmake/Dependencies.cmake`  

This is no longer a blocker. Phase 3 can proceed without a local stub.
