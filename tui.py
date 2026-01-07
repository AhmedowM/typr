from textual.app import App
from textual.events import Key
from textual.widgets import Label
from engine import TypingEngine
from rich.text import Text
import time

class TestApp(App):
    # FILENAME = "sample.txt"  # Configurable filename

    def _read_text_file(self, filename: str) -> str:
        """Reads the main text from the given file."""
        try:
            with open(filename, "r", encoding="utf-8") as file:
                return file.read()
        except FileNotFoundError:
            return "Error: File not found."
        except Exception as e:
            return f"Error: {e}"

    def _format_string_snapshot(self, correct: bool) -> Text:
        completed, current, remaining = self.Engine.get_string_snapshot(max_size=20)
        text = Text()
        if completed:
            text.append(completed)
        if current:
            style = "white on green" if correct else "white on red"
            text.append(current, style=style)
        if remaining:
            text.append(remaining)
        return text

    def __init__(self, filename: str | None = None, timeout: int = 50):
        super().__init__()
        self.filename = filename or "sample.txt"
        self.Text = self._read_text_file(self.filename)
        self.Engine = TypingEngine(self.Text, timeout)

    async def on_mount(self):
        self.timer = self.set_interval(0.1, self.update_ui)

    def compose(self):
        # initial = self.Engine.get_string_snapshot(max_size=20)
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
            # format = f'Correct: {correct}\n'
            # format += f'Timestamp: {timestamp}\nEvent time: {event.time*1e9}'
            format = f'Correct: {stats.correct_chars}   Total: {stats.total_chars}'
            format += f'\nAccuracy: {(stats.accuracy or 0)*100:.0f}%'
            format += f'\nElapsed: {(max(stats.elapsed or 0,0)):.1f}s'
            format += f'\nSpeed: {(stats.speed or 0):.2f}CPM'
            label2.content = format
        # string = f'{event.character} - {event.key} - {event.name}'
        # label.content = string

app = TestApp()
app.run()