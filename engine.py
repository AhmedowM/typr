import time
import logging
from typing import Tuple, Optional
from dataclasses import dataclass
from enum import Enum, auto

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
class Stats:
    elapsed: Optional[float] = None
    accuracy: Optional[float] = None
    speed: Optional[float] = None
    raw_speed: Optional[float] = None

    def __init__(self, _int_st: _Stats | None = None):
        if _int_st is None:
            _int_st = _Stats()
        self.correct_chars = _int_st.correct_chars
        self.total_chars = _int_st.total_chars
        self.elapsed = None
        self.accuracy = None
        self.speed = None
        self.raw_speed = None

class State(Enum):
    IDLE = auto(),
    STOPPED = auto(),
    TIMEOUT = auto(),
    COMPLETED = auto()

# Main engine implementation
class TypingEngine:
    # Internals
    _string: str
    _current_pos: int
    _running: bool
    _stats: _Stats

    _timeout: float | None
    _stop_when_timeout: bool
    _state: State
    
    _is_current_char_correct: bool = True
    
    _default_file: str
    _default_string: str = "Lorem ipsum dolor sit amet, consectetur adipiscing elit."
    
    _logger: logging.Logger

    # Helpers
    def _is_correct_key(self, key: str | None) -> bool | None:
        if key is None or self._current_pos >= len(self._string):
            return None
        return key == self._string[self._current_pos]

    def _increment(self, is_correct: bool) -> None: # advance to next char
        if is_correct:
            self._current_pos += 1
            self._stats.correct_chars += 1
        self._stats.total_chars += 1

    def _read_text_file(self, filename: str) -> tuple[str, int]:
        """Reads the main text from the given file."""
        try:
            with open(filename, "r", encoding="utf-8") as file:
                return file.read(), 0
        except FileNotFoundError:
            self._logger.error(f"File not found: {filename}")
            self._logger.error(f"Using default file instead: '{self._default_file}'") if not filename == self._default_file else self._logger.error("Using default text instead.")
            return "Error: File not found.", 1
        except Exception as e:
            self._logger.error(f"Error reading file {filename}: {e}")
            self._logger.error(f"Using default file instead: '{self._default_file}'") if not filename == self._default_file else self._logger.error("Using default text instead.")
            return f"Error: {e}", 2

    # Constructor
    def __init__(self, string: str = "", timeout: float | None = None, default_file: str = "sample.txt", log_level: int = logging.INFO, log_file: str = "typing_engine.log"):
        self._logger = logging.getLogger(__name__)
        self._logger.setLevel(log_level)
        for handler in self._logger.handlers[:]:
            self._logger.removeHandler(handler)
        file_handler = logging.FileHandler(log_file)
        formatter = logging.Formatter('%(name)s - %(asctime)s - %(levelname)s - %(message)s')
        file_handler.setFormatter(formatter)
        self._logger.addHandler(file_handler)
        self._logger.info(f"Initializing TypingEngine with string of length {len(string)} and timeout {timeout}")
        self._string = string
        self._timeout = timeout
        self._current_pos = 0
        self._running = False
        self._stats = _Stats()
        self._stop_when_timeout: bool = True
        self._default_file = default_file
        self._state = State.IDLE

    # Setters
    def set_string(self, filename: str) -> str: # To reuse the engine
        string, ec = self._read_text_file(filename)
        if ec != 0:
            string, ec = self._read_text_file(self._default_file)
        if ec != 0:
            string = self._default_string
        self._logger.debug(f"Setting new string of length {len(string)}")
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
    def get_state(self):
        return self._state

    def get_string_snapshot(self, max_size: int) -> Tuple[str, str, str]:
        half_size = max_size // 2

        start_index = max(0, self._current_pos - half_size)
        end_index = min(len(self._string), self._current_pos + half_size + 1)

        completed = self._string[start_index:self._current_pos].rjust(half_size)
        current = self._string[self._current_pos] if self._current_pos < len(self._string) else " "
        remaining = self._string[self._current_pos + 1:end_index].ljust(half_size-1)

        return (completed, current, remaining)

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
            ret.raw_speed = 60 * (ret.total_chars or 0)
            if ret.elapsed and ret.elapsed > 0:
                ret.speed /= ret.elapsed
                ret.raw_speed /= ret.elapsed
            else:
                ret.speed = 0.0
                ret.raw_speed = 0.0
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
        return self._state in (
            State.TIMEOUT,
            State.STOPPED,
            State.COMPLETED
        )

    def is_correct(self):
        return self._is_current_char_correct

    # Manipulators
    def start(self, *args) -> float | None:
        self._logger.info("Starting TypingEngine")
        if not self._running: # Don't touch if already running
            self._stats.start_time = args[0] or time.perf_counter_ns()
            self._running = True
            return self._stats.start_time

    def stop(self, end_time: float | None, state: State = State.STOPPED) -> float | None:
        if self._running: # Don't touch if already stopped
            self._logger.info("Stopping TypingEngine")
            self._stats.end_time = end_time or time.perf_counter_ns()
            self._running = False
            self._state = state
            self._logger.info(f"Typing session ended. Total chars: {self._stats.total_chars}, Correct chars: {self._stats.correct_chars}, Reason: {self._state.name}")
            return self._stats.end_time

    def tick(self, now: float) -> None:
        if self._running and self._stop_when_timeout and self.is_timeout(now):
            stop_time = self.get_stop_time() or now
            self.stop(stop_time, State.TIMEOUT)

    def process_key(self, key: str | None) -> Tuple[bool | None,float]:
        is_correct = self._is_correct_key(key)
        pressed_time = time.perf_counter_ns()
        self._is_current_char_correct = is_correct if is_correct is not None else False

        self._logger.debug(f"Processing key: {key}, Correct: {is_correct}, Time: {pressed_time}")

        if not self._running and self._current_pos == 0 and is_correct:
            self.start(pressed_time)
            self._increment(is_correct)
        elif self._running:
            if is_correct is not None: self._increment(is_correct)
            if self._current_pos >= len(self._string):
                self.stop(pressed_time, State.COMPLETED)

        return (is_correct, pressed_time)
