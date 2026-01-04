import time
from typing import Tuple

# Internal stats
class _Stats:
    total_chars: int
    correct_chars: int
    start_time: float | None
    end_time: float | None
    def __init__(self):
        self.total_chars = 0
        self.correct_chars = 0
        self.start_time = None
        self.end_time = None
    
# Stats for end-user
class Stats:
    elapsed: float | None
    accuracy: float | None
    speed: float | None
    
    def __init__(self, _int_st: _Stats | None = None):
        if _int_st is None:
            _int_st = _Stats()
        self.correct_chars = _int_st.correct_chars
        self.total_chars = _int_st.total_chars
        self.elapsed = None
        self.accuracy = None
        self.speed = None

# Main engine implementation
class TypingEngine:
    # Internals
    _string: str
    _current_pos: int
    _running: bool
    _stats: _Stats
    _timeout: float | None
    _stop_when_timeout: bool

    # Helpers
    def _is_correct_key(self, key: str | None) -> bool | None:
        if key is None: return None
        if self._current_pos >= len(self._string): return False
        return key == self._string[self._current_pos]

    def _increment(self, is_correct: bool) -> None: # advance to next char
        if is_correct:
            self._current_pos += 1
            self._stats.correct_chars += 1
        self._stats.total_chars += 1

    # Constructor
    def __init__(self, *args):
        self._string = args[0] or ""
        self._timeout = args[1] or None
        self._current_pos = 0
        self._running = False
        self._stats = _Stats()
        self._stop_when_timeout: bool = True

    # Setters
    def set_string(self, string: str) -> str: # To reuse the engine
        self.set_next_string(string)
        self._stats = _Stats()
        return string

    def set_next_string(self, string: str) -> str: # For later implementation of continuous larger input
        self._string = string
        self._current_pos = 0
        return string

    def set_timeout(self, timeout: float) -> float:
        self._timeout = timeout
        return timeout
    
    # Getters
    def get_string_snapshot(self) -> Tuple[str,str,str]:
        completed: str = self._string[:self._current_pos]
        current: str = self._string[self._current_pos] if self._current_pos < len(self._string) else ""
        remaining: str = self._string[self._current_pos+1:] if self._current_pos < len(self._string) else ""
        return (completed,current,remaining)

    def get_stop_time(self) -> float | None:
        if self._stats.start_time and self._timeout:
            if self._stats.end_time: return self._stats.end_time
            # Mark end_time early to avoid recalculation
            self._stats.end_time = (self._stats.start_time + self._timeout*1e9)
            return self._stats.end_time
        return None

    def get_stats(self, real_time: bool = False, timestamp: float | None = None) -> Stats | None:
        if not self._running or real_time:
            res = self._stats
            ret = Stats(self._stats) #end-user stats
            if real_time and timestamp is not None and self._running:
                ret.elapsed = (timestamp - (res.start_time or 0.0)) / 1e9
            else:
                ret.elapsed = ((res.end_time or 0.0) - (res.start_time or 0.0)) / 1e9
            ret.accuracy = (ret.correct_chars or 0) / (ret.total_chars or 1)
            ret.speed = 60 * (ret.correct_chars or 0)
            if ret.elapsed > 0:
                ret.speed /= ret.elapsed
            else:
                ret.speed = 0.0
            return ret
        return None

    # States
    def is_running(self) -> bool:
        return self._running
    
    def is_timeout(self, pressed_time: float) -> bool:
        if self._stats.start_time:
            elapsed = (pressed_time - (self._stats.start_time or 0.0)) / 1e9
            if self._timeout:
                return elapsed >= self._timeout
            else: return False
        else: # Happens when engine is not started yet, E.g first key press is incorrect
            return False
        
    def is_finished(self):
        return self._current_pos > 0 and not self._running

    # Manipulators        
    def start(self, *args) -> float | None:
        if not self._running: # Don't touch if already running
            self._stats.start_time = args[0] or time.perf_counter_ns()
            self._running = True
            return self._stats.start_time

    def stop(self, *args) -> float | None:
        if self._running: # Don't touch if already stopped
            self._stats.end_time = args[0] or time.perf_counter_ns()
            self._running = False
            return self._stats.end_time

    def process_key(self, key: str | None, pressed_time: float) -> bool:
        is_correct = self._is_correct_key(key)
        if is_correct is None: return False
        stop_time = pressed_time # should be last keypress time if finished before timeout
        is_last = False
        should_stop = False

        if not self._running: # First correct key press
            if self._current_pos == 0 and is_correct:
                self.start(pressed_time)
                self._increment(is_correct)
        else:
            if self._stop_when_timeout:
                if self.is_timeout(pressed_time):
                    should_stop = True
                    stop_time = self.get_stop_time() or stop_time
            if not should_stop:
                self._increment(is_correct)

            is_last = (self._current_pos >= len(self._string))

        if is_last or should_stop:
            self.stop(stop_time)
        
        return is_correct