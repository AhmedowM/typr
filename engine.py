import time
import logging
from typing import Tuple, Optional
from dataclasses import dataclass

logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s', filename='typing_engine.log')

# Internal stats
@dataclass
class _Stats:
    total_chars: int
    correct_chars: int
    start_time: float | None
    end_time: float | None
    def __init__(self):
        self.total_chars = 0
        self.correct_chars = 0
        self.start_time: Optional[float] = None
        self.end_time: Optional[float] = None

# Stats for end-user
@dataclass
class Stats:
    elapsed: Optional[float] = None
    accuracy: Optional[float] = None
    speed: Optional[float] = None

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
    def _is_correct_key(self, key: str | None) -> bool:
        if key is None or self._current_pos >= len(self._string):
            return False
        return key == self._string[self._current_pos]

    def _increment(self, is_correct: bool) -> None: # advance to next char
        if is_correct:
            self._current_pos += 1
            self._stats.correct_chars += 1
        self._stats.total_chars += 1

    # Constructor
    def __init__(self, string: str = "", timeout: float | None = None):
        logging.debug(f"Initializing TypingEngine with string of length {len(string)} and timeout {timeout}")
        self._string = string
        self._timeout = timeout
        self._current_pos = 0
        self._running = False
        self._stats = _Stats()
        self._stop_when_timeout: bool = True

    # Setters
    def set_string(self, string: str) -> str: # To reuse the engine
        logging.debug(f"Setting new string of length {len(string)}")
        self.set_next_string(string)
        self._stats = _Stats()
        return string

    def set_next_string(self, string: str) -> str: # For later implementation of continuous larger input
        logging.debug(f"Setting next string of length {len(string)}")
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
        if self._stats.end_time:
            return self._stats.end_time
        # Mark end_time early to avoid recalculation
        if self._stats.start_time and self._timeout:
            self._stats.end_time = self._stats.start_time + self._timeout*1e9
        return self._stats.end_time

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
            if ret.elapsed and ret.elapsed > 0:
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
        logging.debug("Starting TypingEngine")
        if not self._running: # Don't touch if already running
            self._stats.start_time = args[0] or time.perf_counter_ns()
            self._running = True
            return self._stats.start_time

    def stop(self, *args) -> float | None:
        logging.debug("Stopping TypingEngine")
        if self._running: # Don't touch if already stopped
            self._stats.end_time = args[0] or time.perf_counter_ns()
            self._running = False
            return self._stats.end_time

    def process_key(self, key: str | None, pressed_time: float) -> bool:
        is_correct = self._is_correct_key(key)
        stop_time = pressed_time

        if not self._running and self._current_pos == 0 and is_correct:
            self.start(pressed_time)
            self._increment(is_correct)
        elif self._running:
            if self._stop_when_timeout and self.is_timeout(pressed_time):
                stop_time = self.get_stop_time() or stop_time
                self.stop(stop_time)
            else:
                self._increment(is_correct)
                if self._current_pos >= len(self._string):
                    self.stop(stop_time)

        return is_correct
