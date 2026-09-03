import argparse
import subprocess
import sys
import os
from pathlib import Path

def main() -> None:
    parser = argparse.ArgumentParser(description="Convert an image to ASCII art")
    parser.add_argument("input", help="Path to the input image")
    parser.add_argument("-c", "--cell", type=int, default=8, help="Sampling cell size (default: 8)")
    parser.add_argument("-r", "--render-size", type=int, default=6, help="Output glyph size in pixels (default: 6)")
    parser.add_argument("-o", "--output", default="output.png", help="Output PNG filename (default: output.png)")
    args = parser.parse_args()

    args.output = str(Path(args.output).with_suffix(".png"))




if __name__ == "__main__":
    main()

