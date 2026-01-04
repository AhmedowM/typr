from textual.app import App
from textual.events import Key
from textual.widgets import Label
from engine import TypingEngine
from rich.text import Text

class TestApp(App):
    def _format_string_snapshot(self, correct: bool) -> Text:
        completed, current, remaining = self.Engine.get_string_snapshot()
        print_obj = Text(completed)
        if correct:
            print_obj.append(f'{current}',style='white on green')
        else:
            print_obj.append(f'{current}',style='white on red')
        print_obj.append(remaining)
        return print_obj

    def __init__(self):
        super().__init__()
        self.Text = "stats = self.Engine.get_stats(real_time=True, timestamp=timestamp)"
        self.Engine = TypingEngine(self.Text,20.0)

    def compose(self):
        yield Label(self.Text,id="label")
        yield Label("",id="stats")
    
    def on_key(self, event: Key):
        timestamp = event.time*1e9
        correct = self.Engine.process_key(event.character,timestamp)
        stats = self.Engine.get_stats(real_time=True, timestamp=timestamp)
        label1 = self.query_one("#label",Label)
        label2 = self.query_one("#stats",Label)        
        label1.content = self._format_string_snapshot(correct)

        if stats is not None:
            # format = f'Correct: {correct}\n'
            # format += f'Timestamp: {timestamp}\nEvent time: {event.time*1e9}'
            format = f'Correct: {stats.correct_chars}   Total: {stats.total_chars}'
            format += f'\nAccuracy: {(stats.accuracy or 0)*100:.0f}%'
            format += f'\nElapsed: {(max(stats.elapsed or 0,0)):.2f}s'
            format += f'\nSpeed: {(stats.speed or 0):.2f}CPM'
            label2.content = format
        # string = f'{event.character} - {event.key} - {event.name}'
        # label.content = string

app = TestApp()
app.run()