# engine.py

# in class TypingEngine
def get_string_snapshot(self) -> Tuple[str,str,str]: # removed explicit getters
    completed: str = self._string[:self._current_pos]
    current: str = self._string[self._current_pos] if self._current_pos < len(self._string) else ""
    remaining: str = self._string[self._current_pos+1:] if self._current_pos < len(self._string) else ""
    return (completed,current,remaining)


# textual_test.py
# in class TestApp(App):
def _format_string_snapshot(self, correct: bool) -> Text:
    completed, current, remaining = self.Engine.get_string_snapshot()
    print_obj = Text(completed)
    if correct:
        print_obj.append(f'{current}',style='white on green')
    else:
        print_obj.append(f'{current}',style='white on red')
    print_obj.append(remaining)
    return print_obj

def on_key(self, event: Key):
    timestamp = event.time*1e9
    correct = self.Engine.process_key(event.character,timestamp)
    stats = self.Engine.get_stats(True, timestamp)
    label1 = self.query_one("#label",Label)
    label2 = self.query_one("#stats",Label)        
    label1.content = self._format_string_snapshot(correct)

    if stats is not None:
        format = f'Correct: {stats.correct_chars}   Total: {stats.total_chars}'
        format += f'\nAccuracy: {(stats.accuracy or 0)*100:.0f}%'
        format += f'\nElapsed: {(max(stats.elapsed or 0,0)):.2f}s'
        format += f'\nSpeed: {(stats.speed or 0):.2f}CPM'
        label2.content = format