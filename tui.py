from textual.app import App
from textual.events import Key
from textual.widgets import Label
from engine import TypingEngine
from rich.text import Text
import time

class TestApp(App):
    FILENAME = "tests.txt"

    def _format_string_snapshot(self, correct: bool) -> Text:
        completed, current, remaining = self.Engine.get_string_snapshot(max_size=20)
        text = Text()
        if completed:
            text.append(completed)
        if current:
            style = "black on green" if correct else "white on red"
            text.append(current, style=style)
        if remaining:
            text.append(remaining)
        return text

    def __init__(self, filename: str | None = None, timeout: int = 50):
        super().__init__()
        self.filename = filename or self.FILENAME
        self.Engine = TypingEngine()
        self.Engine.set_string(self.filename)

    async def on_mount(self):
        self.timer = self.set_interval(0.1, self.update_ui)

    def compose(self):
        initial = self._format_string_snapshot(correct=True)
        yield Label(initial,id="label")
        yield Label("",id="stats")
    
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
        label2 = self.query_one("#stats",Label)
        label1.content = self._format_string_snapshot(correct)

        if stats is not None:
            # correct_chars = stats.correct_chars
            # total_chars = stats.total_chars
            elapsed = max(stats.elapsed or 0, 0)
            accuracy = (stats.accuracy or 0) * 100
            speed_cpm = stats.speed or 0
            speed_wpm = speed_cpm / 5
            raw_speed_cpm = stats.raw_speed or 0
            raw_speed_wpm = raw_speed_cpm / 5
            format = f'\nElapsed: {elapsed:.1f}s'
            format += f'\nAccuracy: {accuracy:.0f}%'
            format += f'\nSpeed: {speed_wpm:.2f}WPM'
            format += f'\nRaw Speed: {raw_speed_wpm:.2f}WPM'
            label2.content = format

app = TestApp()
app.run()