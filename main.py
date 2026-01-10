import tui
import argparse

if __name__=="__main__":
    parser = argparse.ArgumentParser(description="Typing App")
    parser.add_argument("-f", "--file", type=str, help="Path to the text file to use")
    parser.add_argument("-t", "--timeout", type=int, default=30, help="Timeout in seconds")
    args = parser.parse_args()

    app = tui.TypingApp(filename=args.file, timeout=args.timeout)
    app.run()