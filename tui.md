# Typr — TUI Design Specification

This document describes the terminal user interface for the Typr typing tester in a framework-agnostic way, suitable as a reference for implementing the TUI in any language or terminal library.

---

## 1. Application Overview

The application is a single-window terminal program composed of **5 screens** that share a common chrome (header bar + footer bar). Only one screen is visible at a time. A central **app controller** manages screen transitions, holds reference to the typing engine, and provides actions that screens can invoke (mode cycling, timeout cycling, content loading).

### Common Chrome

| Element | Position | Behavior |
|---|---|---|
| **Header** | Top edge of terminal | Always visible. Background: dark surface. Text: bright cyan, bold. |
| **Footer** | Bottom edge of terminal | Always visible. Background: dark surface. Text: dim gray. Displays keybinding hints. |

### Color Palette

| Usage | Color | Hex |
|---|---|---|
| Screen background | Near-black | `#121212` |
| Container/card backgrounds | Dark gray | `#1e1e1e` |
| Borders (cards) | Medium gray | `#333333`, `#444444` |
| Primary text | Light gray/white | `#e0e0e0` |
| Accent / headings | Cyan | `#00d7ff` |
| Live stat values | Green | `#00ff00` |
| Personal best indicator | Orange | `#ffaa00` |
| Labels / secondary text | Dim gray | `#555555`, `#888888` |
| Error feedback | Red | `#ff0000` |
| Success header | Green | `#00ff00` |
| Timeout header | Red | `#ff0000` |

---

## 2. Screen Navigation (Routing)

The app maintains a **screen stack**. Navigation replaces the top screen (not pushes onto a stack) — i.e., only one screen is ever visible. The routing map:

| Route Name | Screen | Triggered By |
|---|---|---|
| `main` | MainScreen | App startup, `Ctrl+G` from any screen, "START PRACTICE" button navigation |
| `typing` | TypingScreen | "START PRACTICE" button, restart action, next-content action |
| `results` | ResultsScreen | Auto-navigated when session completes or times out |
| `history` | HistoryScreen | "History" button from main menu |
| `stats` | StatsScreen | "Stats" button from main menu |

When navigating to a results screen, the controller passes the final **Stats** object and **State** enum to the screen constructor.

### Global Keybindings (always active)

| Key | Action |
|---|---|
| `Ctrl+Q` | Quit the application |
| `Ctrl+G` | Return to main menu |
| `Ctrl+C` | Quit (hidden binding) |

---

## 3. Screen-by-Screen Specification

### 3.1 MainScreen (Landing / Home)

**Layout** (vertical stack, center-aligned):

```
┌─ Header ──────────────────────────────────────────┐
│                                                     │
│              █▀▀█ █  █ █▄▄█ █  █                   │
│              █▄ █ █  █ █  █ █  █                   │
│              █▄▄█  █▄  █▄▄█  █▄                    │
│                                                     │
│                  version 0.7.2                      │
│                                                     │
│                 ┌───────────────────┐                │
│                 │  START PRACTICE   │                │
│                 └───────────────────┘                │
│                                                     │
│          [History]          [Stats]                  │
│                                                     │
├─ Footer ───────────────────────────────────────────┤
│ Ctrl+Q Quit | Ctrl+G Menu                           │
└─────────────────────────────────────────────────────┘
```

**Components:**

| Component | Type | Details |
|---|---|---|
| Logo area | Static text (4 rows) | Rendered with ASCII block font (see §5). Color: cyan, bold. |
| Version label | Static text | Below logo. Color: dim gray, italic. |
| Start button | Button (3 rows tall, 33 cols wide) | Bold text. Cyan-themed border (`vkey` style). Triggers engine reset + navigate to typing screen. |
| History button | Button (15 cols wide) | Navigates to history screen. |
| Stats button | Button (15 cols wide) | Navigates to stats screen. |

**Interactions:**
- Click/tap any button to navigate (no keyboard bindings on this screen besides globals).

---

### 3.2 TypingScreen (Gameplay)

**Layout** (vertical stack):

```
┌─ Header ──────────────────────────────────────────┐
│                                                     │
│   MODE: [STRICT]       TIMEOUT: [30s]               │
│                                                     │
│   ┌─────────────────────────────────────────────┐   │
│   │                                             │   │
│   │  [rendered text area — see below]           │   │
│   │                                             │   │
│   └─────────────────────────────────────────────┘   │
│                                                     │
│   ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌────────┐│
│   │ 0.0s     │ │ 100%     │ │ 0.00 WPM │ │0.00 WPM││
│   │ TIME     │ │ ACCURACY │ │ SPEED    │ │RAW SPD ││
│   └──────────┘ └──────────┘ └──────────┘ └────────┘│
│                                                     │
├─ Footer ───────────────────────────────────────────┤
│ Ctrl+Q Quit | Ctrl+G Menu | Ctrl+R Restart          │
└─────────────────────────────────────────────────────┘
```

**Settings Bar (below header, above text area):**
- Two labels arranged horizontally, equal width, center-aligned.
- Left: `MODE: [{mode}]` where mode is STRICT or FLOW. Inactive state shows `(Ctrl+S)` hint.
- Right: `TIMEOUT: [{seconds}s]`. Inactive state shows `(Ctrl+T)` hint.
- Container has a rounded border, dark gray background.
- Mode/timeout labels are dim gray.

**Text Display Area:**
- Takes all remaining vertical space (`height: 1fr` — proportional fill).
- Rounded border, `#1e1e1e` background, padding on left/right.
- Content is **left-aligned** during active typing, **center-aligned** in idle state.

**Text Rendering (per-character coloring):**

| Character Position | Style |
|---|---|
| Already typed correctly | Green foreground |
| Already typed incorrectly | Red foreground |
| Current character (cursor) | Black text on white background (inverse cursor) |
| Not yet typed | Dim (gray, low-intensity) |

When the screen is **idle** (before the user starts), the entire text area shows a single centered message: `Press [SPACE] or [ENTER] to start`.

**Stat Boxes (4 boxes, equal width, arranged horizontally):**

Each stat box is a bordered card (`width: 1fr`, equal proportional width) containing:
- A large value label (green, bold): elapsed time (s), accuracy (%), speed (WPM), raw speed (WPM).
- A small italic label below (dim gray): "TIME", "ACCURACY", "SPEED", "RAW SPEED".

**Interactions:**

| Key / Action | Behavior |
|---|---|
| `Space` or `Enter` (idle) | Activate session: reset engine, begin typing |
| `Ctrl+S` (idle only) | Cycle typing mode (strict → flow → strict) |
| `Ctrl+T` (idle only) | Cycle timeout (10s → 15s → 30s → 60s → 120s) |
| `Ctrl+R` | Restart session (reset engine, return to idle) |
| Printable characters | Send to engine for processing |
| `Enter` | Send newline to engine |
| `Backspace` (flow mode only) | Revert last character (decrement cursor, adjust stats) |
| `Backspace` (strict mode) | Ignored |

**Real-time Updates:**
- A **100ms interval timer** fires a screen update callback.
- Each tick: engine timeout check → re-render text with updated cursor position → update 4 stat labels with live values.
- On a **mistake in strict mode**: text area background flashes red for 100ms, then returns to `#1e1e1e`.
- On **session end** (COMPLETED or TIMEOUT state): instantly navigate to results screen.

---

### 3.3 ResultsScreen (Post-Session)

**Layout** (vertical stack):

```
┌─ Header ──────────────────────────────────────────┐
│                                                     │
│              ▄▄█ █▄▄▄ █▄▄█ █▄▄█ █▄▄▄ █▄▄█         │
│               ▄█  █   █    █  █ █    █             │
│              ▄▄█  █▄▄▄ █▄▄█ █▄▄█ █▄▄▄ █            │
│                                                     │
│   ┌─────────────────────────────┐ ┌──────────────┐ │
│   │   🔥 NEW PERSONAL BEST! 🔥  │ │              │ │
│   │                             │ │ Recent       │ │
│   │   ┌──────┐ ┌──────┐        │ │ History       │ │
│   │   │65.2  │ │ 96%  │        │ │              │ │
│   │   │ WPM  │ │Accuracy       │ │ 03-15 65.2.. │ │
│   │   └──────┘ └──────┘        │ │ 03-14 58.1.. │ │
│   │   ┌──────┐ ┌──────┐        │ │ 03-13 72.4.. │ │
│   │   │29.8s │ │67.9  │        │ │ 03-12 50.0.. │ │
│   │   │ Time │ │RawWPM│        │ │ 03-11 61.3.. │ │
│   │   └──────┘ └──────┘        │ │              │ │
│   │                             │ └──────────────┘ │
│   └─────────────────────────────┘                   │
│                                                     │
│   Press [Ctrl+R] Restart | [Ctrl+N] Next | [Ctrl+G]│
│                                                     │
├─ Footer ───────────────────────────────────────────┤
│ Ctrl+Q Quit | Ctrl+G Menu | Ctrl+R Restart | ...    │
└─────────────────────────────────────────────────────┘
```

**Big Header:**
- 4-row ASCII block text.
- If state is COMPLETED: word "completed" in green.
- If state is TIMEOUT: word "timeout" in red.

**Layout (two-column horizontal split):**

| Panel | Width | Contents |
|---|---|---|
| Stats panel | 2/3 of width | Personal best indicator + 2 row of stat boxes |
| History panel | 1/3 of width | "Recent History" label (orange) + scrollable list |

**Personal Best Indicator:**
- Hidden by default (empty label).
- If current WPM >= previous max (and max > 0): shows a flame emoji and "NEW PERSONAL BEST!" in orange (`#ffaa00`).
- Queries the database on mount to fetch the previous best.

**Stat Boxes (2 rows of 2 boxes):**
- Row 1: WPM (large value) + Accuracy (%)
- Row 2: Elapsed time (s) + Raw WPM
- Each box: border, dark background, bold cyan value, dim gray label.

**Interactions:**

| Key | Action |
|---|---|
| `Ctrl+R` | Restart with same text (engine reset → typing screen) |
| `Ctrl+N` | Load next content chunk (engine reset → next_content → typing screen). On failure, show error notification. |
| `Ctrl+G` | Return to main menu |

---

### 3.4 HistoryScreen (Session Browser)

**Layout:**

```
┌─ Header ──────────────────────────────────────────┐
│                                                     │
│   ┌─────────────────────────────────────────────┐   │
│   │   ID │ Date       │ Mode  │ WPM  │ Acc │Tim│   │
│   ├─────┼─────────────┼───────┼──────┼─────┼───┤   │
│   │   1 │ 25-03-15 14 │ Strict│ 65.2 │ 96% │30s│   │
│   │   2 │ 25-03-14 09 │ Flow  │ 58.1 │ 93% │60s│   │
│   │   3 │ 25-03-13 11 │ Strict│ 72.4 │ 98% │30s│   │
│   │   ...                                      │   │
│   └─────────────────────────────────────────────┘   │
│                                                     │
├─ Footer ───────────────────────────────────────────┤
│ Ctrl+Q Quit | Ctrl+G Menu | Delete Delete Entry     │
└─────────────────────────────────────────────────────┘
```

**DataTable:**
- Takes all available height.
- Rounded border, dark background.
- Columns: ID, Date (short format), Mode (capitalized), WPM (1 decimal), Accuracy (%), Time (seconds).
- Row cursor enabled (highlight current row).
- Shows last 50 sessions, most recent first.
- On mount: queries database and populates table.

**Interactions:**

| Key | Action |
|---|---|
| `Escape` or `Ctrl+G` | Return to main menu |
| `Delete` | Delete the currently selected row from database, refresh table, show notification |

---

### 3.5 StatsScreen (Global Analytics)

**Layout:**

```
┌─ Header ──────────────────────────────────────────┐
│                                                     │
│   ┌────────────────┐ ┌────────────────┐ ┌──────────┐│
│   │ Total Sessions │ │ Average WPM    │ │ Personal ││
│   │                │ │                │ │ Best     ││
│   │      12        │ │     61.4       │ │   72.4   ││
│   └────────────────┘ └────────────────┘ └──────────┘│
│                                                     │
│   ┌─────────────────────────────────────────────┐   │
│   │                                             │   │
│   │     [Graph Visualization Coming Soon]       │   │
│   │     (Will feature WPM trend chart)          │   │
│   │                                             │   │
│   └─────────────────────────────────────────────┘   │
│                                                     │
├─ Footer ───────────────────────────────────────────┤
│ Ctrl+Q Quit | Ctrl+G Menu                           │
└─────────────────────────────────────────────────────┘
```

**Summary Row (3 equal-width boxes):**
- Total Sessions: count from database.
- Average WPM: mean WPM across all sessions.
- Personal Best: max WPM across all sessions.
- Each box: border, dark background, bold cyan value, dim gray label.

**Graph Placeholder:**
- Full-width area below summary.
- Bordered, dark background.
- Contains placeholder text: "Graph Visualization Coming Soon (Will feature WPM trend chart)" in dim gray.
- Reserved for future WPM-over-time chart.

**Interactions:**

| Key | Action |
|---|---|
| `Escape` or `Ctrl+G` | Return to main menu |

---

## 4. Input Handling Patterns

### 4.1 Keyboard Event Routing

All keyboard input on the TypingScreen goes through a single handler that:

1. **Checks terminal states first**: If engine is in COMPLETED or TIMEOUT, ignore all input (the screen will auto-navigate to results on the next timer tick).
2. **Checks active flag**: If not yet active, route to the "inactive" handler which only responds to Space/Enter (start), Ctrl+S (cycle mode), Ctrl+T (cycle timeout).
3. **Processing active input**:
   - `Backspace` → only in flow mode, delegates to engine.backspace().
   - Printable characters + `Enter` → delegates to engine.process_key().
   - All other keys → ignored.

### 4.2 Timer-Based Refresh

A recurring 100ms timer drives all real-time updates:
- Engine timeout check via `engine.tick()`
- Text area re-render
- Stat label updates (elapsed, accuracy, speed, raw speed)
- State change detection (navigate to results on completion/timeout)

This avoids polling on every keystroke and decouples display from input.

### 4.3 Visual Error Feedback

In strict mode, when the user hits a wrong key:
1. The text area background immediately changes to red.
2. A one-shot timer resets it back to `#1e1e1e` after 100ms.
3. The incorrect character is rendered in red text (standard per-character styling handles this).

---

## 5. ASCII Block-Letter Rendering

### 5.1 Font Specification

A custom bitmap font maps each lowercase letter (a-z), `!`, and space to a **4-row** array of block characters (`█`, `▀`, `▄`, `▀`, spaces). Each character is rendered as a fixed-width glyph (3–5 columns wide depending on the letter).

### 5.2 Rendering Algorithm

```
function renderBigText(text, style):
    rows = ["", "", "", ""]                   # 4 rows of output
    for each char in text:
        glyph = BLOCK_MAP[char.lower()]       # fallback to space glyph
        for row_index in 0..3:
            rows[row_index] += glyph[row_index] + " "  # append glyph row + spacer
    return join(rows, "\n")                   # combine into 4-line string
```

### 5.3 Usage Locations

| Screen | Text Rendered | Style |
|---|---|---|
| MainScreen | "TYPR" | Cyan, bold |
| ResultsScreen (completed) | "completed" | Green, bold |
| ResultsScreen (timeout) | "timeout" | Red, bold |

---

## 6. Theme / Styling System

The styling uses a CSS-like system targeting widgets by **ID** (`#widget_id`) and **class** (`.class_name`):

### Key Selectors & Their Purpose

| Selector | Target | Key Properties |
|---|---|---|
| `Screen` | All screens | Background `#121212`, vertical layout, centered |
| `#main_container` / `#results_container` etc. | Per-screen root containers | Full width/height, vertical layout |
| `#logo_area` | ASCII logo | Width auto, height 4, center-aligned |
| `#start_button` | Start button | Width 33, height 3, bold, vkey border |
| `#text_container` | Typing text area | 1fr height, rounded border, background `#1e1e1e` |
| `#text_area` | Text content inside | Full width/height, bold, light text |
| `.stat_box` | Stat display cards | Equal width (1fr), rounded border, centered |
| `#elapsed`, `#accuracy` etc. | Stat values | Green, bold, center-aligned |
| `.stat_label` | Stat labels under values | Dim gray, italic |
| `#settings_bar` | Mode/timeout bar | Horizontal layout, background `#1e1e1e`, rounded border |
| `#results_header` | Big result text | Height 8 (4 rows + spacing), center-aligned |
| `.summary_box` | Stats screen summary cards | 1fr width, rounded border, centered |
| `#history_table` | Session history table | 1fr height, rounded border |
| `Header` / `Footer` | Chrome bars | Background `#1e1e1e`, header cyan bold, footer dim gray |

### Layout Concepts

- **`layout: vertical`** — stacks children top-to-bottom.
- **`layout: horizontal`** — arranges children left-to-right.
- **`width: 1fr`** — proportional sizing: divides available space equally among `1fr` siblings.
- **`height: 1fr`** — fills remaining vertical space proportionally.
- **`align: center middle`** — centers children within the container.
- **`content-align: center middle`** — centers the widget's own content.
- **`border: round`** — rounded border lines (`╭╮╰╯│─`).
- **`display: none`** — hides elements (used by `.hidden` class).

---

## 7. Screen Lifecycle

Each screen follows this lifecycle:

1. **Construct** — receive any required data (engine reference, stats, state).
2. **Compose** — declare the widget tree (layout structure with placeholders).
3. **Mount** — populate dynamic content (query DB, render logos, set initial values).
4. **Active** — respond to user input and timer events.
5. **Navigated away** — screen is replaced; no explicit cleanup needed (next screen's compose replaces it).

The app controller (`TypingApp` equivalent) is the single owner of the engine instance. Screens access it through `self.app.engine` (or equivalent parent reference).

---

## 8. Timer Mechanisms

| Timer | Interval | Purpose |
|---|---|---|
| TypingScreen refresh | 100ms | Engine tick, text re-render, stat updates, state-change detection |
| Error shake (strict mode) | 100ms (one-shot) | Reset text area background after error flash |

Timers are stopped implicitly when the screen is replaced.

---

## 9. Data Flow (TUI ↔ Engine)

```
Screen Input Handler
    │
    ▼
App Controller methods:
    engine.process_key(key)
    engine.backspace()
    engine.tick(timestamp)
    engine.reset()
    engine.next_content()
    engine.get_stats(...)
    engine.get_state()
    cycle_mode()
    cycle_timeout()
    │
    ▼
Screen re-renders from engine state:
    engine.string          → text to display
    engine.current_pos     → cursor position
    engine.incorrect_indices → set of incorrect char indices
    engine.get_stats()     → speed, accuracy, elapsed, raw speed
    engine.get_state()     → IDLE, RUNNING, COMPLETED, TIMEOUT, etc.
```

---

## 10. Summary of Key Design Principles

1. **Single window, 5 screens** — one visible at a time, managed by a central router.
2. **Common chrome** — header and footer persist across all screens.
3. **Dark theme** — `#121212`/`#1e1e1e` background, green/cyan accents, rounded borders.
4. **CSS-like styling** — ID-based and class-based selectors with layout, color, border, and alignment properties.
5. **Timer-driven refresh** — 100ms interval for real-time stat updates and timeout detection.
6. **Per-character text coloring** — green (correct), red (incorrect), inverse cursor, dim (untyped).
7. **Screen receives data via constructor** — results screen gets Stats + State; all screens access the engine through the app controller.
8. **No mouse required** — all interactions available via keyboard bindings.
9. **Explicit routing** — no URL/route pattern; methods on the controller switch screens directly.
