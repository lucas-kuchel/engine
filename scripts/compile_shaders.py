#!/usr/bin/env python3

import subprocess
import pathlib
from concurrent.futures import ThreadPoolExecutor, as_completed

SOURCE_DIR = pathlib.Path("assets/shaders/source")
OUTPUT_DIR = pathlib.Path("assets/shaders/bin")

def compile_shader(path: pathlib.Path):
    output = OUTPUT_DIR / (path.name + ".spv")

    cmd = ["glslangValidator", "-V", str(path), "-o", str(output)]
    result = subprocess.run(cmd, capture_output=True, text=True)

    if result.returncode != 0:
        return f"Compilation failed: {path.name} -> {output}:\n{result.stderr.strip()}"
    else:
        return f"Compilation succeeded: {path.name} → {output}"

def main():
    if not SOURCE_DIR.exists():
        print(f"Error: directory not found: {SOURCE_DIR}")
        exit(1)

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    files = [p for p in SOURCE_DIR.glob("*.*") if p.is_file()]
    if not files:
        print("Error: no shader files found")
        return

    print(f"Compiling {len(files)} shaders to {OUTPUT_DIR}...\n")

    with ThreadPoolExecutor(max_workers=8) as executor:
        futures = {executor.submit(compile_shader, path): path for path in files}

        for future in as_completed(futures):
            print(future.result())

    print("\nAll compilations completes")

if __name__ == "__main__":
    main()
