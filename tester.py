from random import randint
from engine import TypingEngine,Stats
import time

expected: str = "My name is Connor\nI am a deviant"
tested: str = "abcdMy name is Connoer\nI am not a devian"
timeout = 10.0

engine = TypingEngine(expected,timeout)
""" Same as:
engine = TypingEngine()
engine.set_string(expected)
engine.set_timeout(timeout)
"""

for key in tested:
    k = key
    if key == " ":
        k = "space"
    elif key == "\n":
        k = "newline"
    pressed_time = time.perf_counter_ns()
    engine.process_key(k,pressed_time)
    if engine.is_timeout(pressed_time):
        engine.stop(engine.get_stop_time())
    time.sleep(randint(1,4)/10)

time.sleep(5) # to simulate user never finished
if engine.is_timeout(time.perf_counter_ns()):
    engine.stop(engine.get_stop_time())

stats = engine.get_stats() or Stats()
print(f'Correct chars: {stats.correct_chars or 0}')
print(f'Total chars:   {stats.total_chars or 0}')
print(f'Elapsed time:  {(stats.elapsed or 0):.2f}s')
print(f'Accuracy:      {(100.0*(stats.accuracy or 0)):.0f}%')
print(f'Speed(CPM):    {(stats.speed or 0):.2f}')
print(f'Speed(WPM):    {((stats.speed or 0)/5):.2f}\n')