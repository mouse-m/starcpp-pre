#!/usr/bin/env python3
import sys
import glob
import subprocess
import os
import time
def run_checker(exe_path, input_data):
    try:
        result = subprocess.run(
            [exe_path],
            input=input_data,
            capture_output=True,
            text=True,
            timeout=5
        )
        return result.stdout.strip()
    except subprocess.TimeoutExpired:
        return "[Timeout]"
    except Exception as e:
        return f"[Error: {str(e)}]"

def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <input_pattern_or_files> <checker>")
        print("The program will exit in 5 seconds... Or you can close it yourself.")
        time.sleep(5)
        sys.exit(1)
    check_exe = sys.argv[-1]
    input_args = sys.argv[1:-1]
    files = []
    for arg in input_args:
        if '*' in arg or '?' in arg:
            matched = glob.glob(arg)
            if matched:
                files.extend(matched)
            else:
                print(f"[Warning] No files found matching: {arg}", file=sys.stderr)
        else:
            if os.path.isfile(arg):
                files.append(arg)
            else:
                print(f"[Warning] File not found: {arg}", file=sys.stderr)
    files.sort()
    if not files:
        print("[Error] No valid input files to process.", file=sys.stderr)
        sys.exit(1)
    for file_path in files:
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
            output = run_checker(check_exe, content)
            filename = os.path.basename(file_path)
            print(f"{filename}: {output}")

        except Exception as e:
            print(f"{os.path.basename(file_path)}: [Error reading file - {str(e)}]")

if __name__ == "__main__":
    main()
