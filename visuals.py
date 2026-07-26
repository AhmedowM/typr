from rich.text import Text

# Mapping for TYPR and COMPLETED/TIMEOUT letters
# Lowercase 4-row block font based on @charmap.txt style
BLOCK_MAP = {
    "a": [
        "    ",
        "█▀▀█",
        "█▄ █",
        "    "
    ],
    "b": [
        "█   ",
        "█▀▀█",
        "█▄▄█",
        "    "
    ],
    "c": [
        "   ",
        "█▀▀",
        "█▄▄",
        "   "
    ],
    "d": [
        "   █",
        "█▀▀█",
        "█▄▄█",
        "    "
    ],
    "e": [
        "    ",
        "█▀▀█",
        "█▄▄ ",
        "    ",
    ],
    "f": [
        "   ",
        "█▀▀",
        "█▀ ",
        "   "
    ],
    "g": [
        "    ",
        "█▀▀█",
        "█▄▄█",
        " ▄▄█"
    ],
    "h": [
        "█   ",
        "█▀▀█",
        "█  █",
        "    "
    ],
    "i": [
        "▀",
        "▄",
        "█",
        " ",
    ],
    "j": [
        " ▀",
        " ▄",
        " █",
        "▄█"
    ],
    "k": [
        "█   ",
        "█▄█▀",
        "█  █",
        "    "
    ],
    "l": [
        "█",
        "█",
        "█",
        " ",
    ],
    "m": [
        "     ",
        "█▀█▀█",
        "█ █ █",
        "     "
    ],
    "n": [
        "    ",
        "█▀▀█",
        "█  █",
        "    "
    ],
    "o": [
        "    ",
        "█▀▀█",
        "█▄▄█",
        "    "
    ],
    "p": [
        "    ",
        "█▀▀█",
        "█▄▄█",
        "█   ",
    ],
    "q": [
        "    ",
        "█▀▀█",
        "█▄▄█",
        "   █"
    ],
    "r": [
        "   ",
        "█▀▀",
        "█  ",
        "   "
    ],
    "s": [
        "   ",
        "█▀▀",
        "▄▄█",
        "   "
    ],
    "t": [
        "█  ",
        "█▀ ",
        "█▄▄",
        "   "
    ],
    "u": [
        "    ",
        "█  █",
        "█▄▄█",
        "    "
    ],
    "v": [
        "    ",
        "█  █",
        "▀▄▄▀",
        "    "
    ],
    "w": [
        "     ",
        "█ █ █",
        "█▄█▄█",
        "     "
    ],
    "x": [
        "    ",
        "▀ ▄▀",
        "▄▀ ▄",
        "    "
    ],
    "y": [
        "    ",
        "█  █",
        "█▄▄█",
        " ▄▄█"
    ],
    "z": [
        "   ",
        "▀▀█",
        "█▄▄",
        "   "
    ],
    "!": [
        "█",
        "█",
        " ",
        "▀"
    ],
    " ": [
        "  ",
        "  ",
        "  ",
        "  "
    ]
}

def get_big_text(word: str, style: str = "bold green") -> Text:
    """Stitches 4-row block characters together horizontally."""
    rows = ["", "", "", ""]
    for char in word.lower():
        char_rows = BLOCK_MAP.get(char, BLOCK_MAP[" "])
        for i in range(4):
            rows[i] += char_rows[i] + " "
    
    return Text("\n".join(rows), style=style)
