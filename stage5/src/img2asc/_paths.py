import sys
from pathlib import Path

_PKG_DIR = Path(__file__).parent
BINARY_PATH = _PKG_DIR / "bin" / ("img2asc-bin.exe" if sys.platform == "win32" else "img2asc-bin")
FONT_PATH = _PKG_DIR / "bin" / "Px437_IBM_VGA_9x16.ttf"
