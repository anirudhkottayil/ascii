import subprocess
import os
import tempfile
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor, as_completed
from ._paths import BINARY_PATH, FONT_PATH

def get_frame_rate(video_path):
    result = subprocess.run(
        ["ffprobe", "-v", "error", "-select_streams", "v:0",
         "-show_entries", "stream=r_frame_rate",
         "-of", "default=noprint_wrappers=1:nokey=1", str(video_path)],
        capture_output=True, text=True, check=True)
    return result.stdout.strip()

def convert_frame(in_frame, out_frame, cell, render_size):
    cmd = [str(BINARY_PATH), str(in_frame), str(out_frame), str(cell), str(render_size), str(FONT_PATH)]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"Failed on {in_frame.name}: {result.stderr}")

def convert_video(input_video, output_video, cell, render_size):
    frame_rate = get_frame_rate(input_video)
    num_workers = os.cpu_count() or 4
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        in_dir, out_dir = tmp / "in", tmp / "out"
        in_dir.mkdir(); out_dir.mkdir()
        print("Extracting frames...")
        subprocess.run(["ffmpeg", "-i", str(input_video), str(in_dir / "frame_%06d.png")],
                        check=True, capture_output=True)
        frames = sorted(in_dir.glob("frame_*.png"))
        print(f"Converting {len(frames)} frames on {num_workers} workers...")
        with ThreadPoolExecutor(max_workers=num_workers) as pool:
            futures = []
            for f in frames:
                out_frame = out_dir / f.name.replace("frame_", "out_")
                futures.append(pool.submit(convert_frame, f, out_frame, cell, render_size))
            done = 0
            for future in as_completed(futures):
                future.result()
                done += 1
                print(f"  {done}/{len(frames)}", end="\r")
        print()
        print("Reassembling with audio...")
        subprocess.run([
            "ffmpeg", "-framerate", frame_rate, "-i", str(out_dir / "out_%06d.png"),
            "-i", str(input_video),
            "-map", "0:v", "-map", "1:a?",
            "-c:v", "libx264", "-pix_fmt", "yuv420p", "-c:a", "aac",
            "-shortest", str(output_video)
        ], check=True)
    print(f"Done: {output_video}")
