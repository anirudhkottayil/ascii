import argparse
import subprocess
import sys
import os
from pathlib import Path

def find_binary():
    matches = list((Path(__file__).parent / "bin").glob("img2asc-bin*"))
    if not matches:
        sys.exit("Bundled binary missing — this install may be corrupted.")
    return matches[0]

def main() -> None:
    parser = argparse.ArgumentParser(description="Convert an image to ASCII art")
    parser.add_argument("input", help="Path to the input image")
    parser.add_argument("-c", "--cell", type=int, default=8, help="Sampling cell size (default: 8)")
    parser.add_argument("-r", "--render-size", type=int, default=6, help="Output glyph size in pixels (default: 6)")
    parser.add_argument("-o", "--output", default="output.png", help="Output PNG filename (default: output.png)")
    args = parser.parse_args()

    args.output = str(Path(args.output).with_suffix(".png"))

    binary = find_binary()
    cmd = [str(binary), str(Path(args.input).resolve()), str(Path(args.output).resolve()),
           str(args.cell), str(args.render_size)]
    result = subprocess.run(cmd, capture_output=True, text=True, cwd=binary.parent)
    if result.returncode != 0:
        sys.exit(f"Conversion failed: {result.stderr}")
    print(f"Saved to {args.output}")


if __name__ == "__main__":
    main()

