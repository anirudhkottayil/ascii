import argparse
import subprocess
import sys
from pathlib import Path

_PKG_DIR = Path(__file__).parent
BINARY_PATH = _PKG_DIR / "bin" / ("img2asc-bin.exe" if sys.platform == "win32" else "img2asc-bin")
FONT_PATH = _PKG_DIR / "bin" / "Px437_IBM_VGA_9x16.ttf"

def main() -> None:
    parser = argparse.ArgumentParser(description="Convert an image to ASCII art")
    parser.add_argument("input", help="Path to the input image")
    parser.add_argument("-c", "--cell", type=int, default=8, help="Sampling cell size (default: 8)")
    parser.add_argument("-r", "--render-size", type=int, default=6, help="Output glyph size in pixels (default: 6)")
    parser.add_argument("-o", "--output", default="output.png", help="Output PNG filename (default: output.png)")
    args = parser.parse_args()

    args.output = str(Path(args.output).with_suffix(".png"))

    cmd = [str(BINARY_PATH), str(Path(args.input).resolve()), str(Path(args.output).resolve()), str(args.cell), str(args.render_size), str(FONT_PATH)]

    result = subprocess.run(cmd, capture_output=True, text=True, cwd=BINARY_PATH.parent)
    if result.returncode != 0:
        sys.exit(f"Conversion failed: {result.stderr}")
    print(f"Saved to {args.output}")


if __name__ == "__main__":
    main()

