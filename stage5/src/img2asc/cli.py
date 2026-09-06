import argparse
import subprocess
import sys
import shutil
from pathlib import Path
from ._paths import BINARY_PATH, FONT_PATH

def main() -> None:
    parser = argparse.ArgumentParser(description="Convert an image to ASCII art")
    parser.add_argument("input", help="Path to the input image")
    parser.add_argument("-c", "--cell", type=int, default=8, help="Sampling cell size (default: 8)")
    parser.add_argument("-r", "--render-size", type=int, default=6, help="Output glyph size in pixels (default: 6)")
    parser.add_argument("-o", "--output", default="output.png", help="Output PNG filename (default: output.png)")
    parser.add_argument("--video", action="store_true", help="Treat input as a video")
    args = parser.parse_args()
    args.output = str(Path(args.output).with_suffix(".mp4" if args.video else ".png"))

    if args.video:
        if shutil.which("ffmpeg") is None or shutil.which("ffprobe") is None:
            sys.exit("ffmpeg (with ffprobe) is required for --video but wasn't found on your PATH. Install it from https://ffmpeg.org/download.html and try again.")
        from .video import convert_video
        convert_video(Path(args.input).resolve(), Path(args.output), args.cell, args.render_size)
        return

    cmd = [str(BINARY_PATH), str(Path(args.input).resolve()), str(Path(args.output).resolve()), str(args.cell), str(args.render_size), str(FONT_PATH)]

    result = subprocess.run(cmd, capture_output=True, text=True, cwd=BINARY_PATH.parent)
    if result.returncode != 0:
        sys.exit(f"Conversion failed: {result.stderr}")
    print(f"Saved to {args.output}")


if __name__ == "__main__":
    main()

