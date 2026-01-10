from textual.app import App
from textual.events import Key
from textual.widgets import Label, Static
from textual.containers import Container, Horizontal
from engine import TypingEngine
from rich.text import Text
import time

class TypingApp(App):
    FILENAME = "tests.txt"
    CSS_PATH = "style.tcss"

    def _format_string_snapshot(self, correct: bool, max_size: int = 20) -> Text:
        completed, current, remaining = self.Engine.get_string_snapshot(max_size)
        text = Text()
        if completed:
            text.append(completed)
        if current:
            style = "black on green" if correct else "white on red"
            text.append(current, style=style)
        if remaining:
            text.append(remaining)
        return text

    def __init__(self, filename: str | None = None, timeout: int = 30):
        super().__init__()
        self.filename = filename or self.FILENAME
        self.Engine = TypingEngine()
        self.Engine.set_string(self.filename)
        self.Engine.set_timeout(timeout)

    async def on_mount(self):
        self.timer = self.set_interval(0.1, self.update_ui)

    def compose(self):
        initial = self._format_string_snapshot(correct=True)
        yield Container(
            Static(id="spacer1"),
            Horizontal(
                Static(id="l_spacer1"),
                Label(initial,id="label"),
                Static(id="l_spacer2")
            ),
            Static(id="spacer2"),
            Horizontal(
                Label("Elapsed", id="elapsed"),
                Label("Accuracy", id="accuracy"),
                Label("Speed", id="speed"),
                Label("Raw Speed", id="raw_speed"),
                id="stats_container"
            ),
            id="main_container"
        )
    
    def on_key(self, event: Key):
        key_to_process = event.character
        if event.key == "enter":
            key_to_process = "\n"
        if not event.is_printable:
            return
        timestamp = event.time*1e9
        correct = self.Engine.process_key(key_to_process,timestamp)
        self.update_ui(correct, timestamp)

    def update_ui(self, correct: bool | None = None, timestamp: float | None = None):
        if timestamp is None:
            timestamp = time.perf_counter_ns()
        if correct is None:
            correct = self.Engine.is_correct()
        stats = self.Engine.get_stats(real_time=True, timestamp=timestamp)
        label1 = self.query_one("#label",Label)
        label1.content = self._format_string_snapshot(correct, label1.size.width-6)

        if stats is not None:
            elapsed = max(stats.elapsed or 0, 0)
            accuracy = (stats.accuracy or 0) * 100
            speed_cpm = stats.speed or 0
            speed_wpm = speed_cpm / 5
            raw_speed_cpm = stats.raw_speed or 0
            raw_speed_wpm = raw_speed_cpm / 5

            elapsed_label = self.query_one("#elapsed",Label)
            accuracy_label = self.query_one("#accuracy",Label)
            speed_label = self.query_one("#speed",Label)
            raw_speed_label = self.query_one("#raw_speed",Label)

            elapsed_label.content = f'{elapsed:.1f}s'
            accuracy_label.content = f'{accuracy:.0f}%'
            speed_label.content = f'{speed_wpm:.2f}WPM'
            raw_speed_label.content = f'{raw_speed_wpm:.2f}WPM'
            
            elapsed_label.border_subtitle = f'Elapsed Time'
            accuracy_label.border_subtitle = f'Accuracy'
            speed_label.border_subtitle = f'Speed'
            raw_speed_label.border_subtitle = f'Raw Speed'