import sys
import subprocess
import threading
import time

stop_flag = False

def forward_thread(src_name, src_proc, dst_proc):
    global stop_flag
    try:
        for line in iter(src_proc.stdout.readline, ''):
            if stop_flag:
                break
            if not line:
                break
            
            try:
                clean_line = line.rstrip('\r\n')
            except:
                clean_line = line
            
            sys.stderr.write(f"{src_name}: {clean_line}\n")
            sys.stderr.flush()
            
            try:
                dst_proc.stdin.write(line)
                dst_proc.stdin.flush()
            except BrokenPipeError:
                stop_flag = True
                break
    except Exception:
        pass
    finally:
        stop_flag = True

def main():
    global stop_flag
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <your_program> <interact_lib> [interact_input]", file=sys.stderr)
        return 1
    prog = sys.argv[1]
    lib = sys.argv[2]
    interact_input = ""
    if len(sys.argv) >= 4:
        interact_input = sys.argv[3]
    else:
        sys.stderr.write("Answer: ")
        sys.stderr.flush()
        try:
            interact_input = sys.stdin.readline().strip()
        except (EOFError, KeyboardInterrupt):
            return 1
    if not interact_input:
        sys.stderr.write("No input provided. Exiting...\n")
        return 1
    l_proc = subprocess.Popen(
        lib, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, 
        shell=True, text=True, bufsize=1
    )
    
    p_proc = subprocess.Popen(
        prog, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, 
        shell=True, text=True, bufsize=1
    )
    try:
        l_proc.stdin.write(interact_input + "\n")
        l_proc.stdin.flush()
    except BrokenPipeError:
        print("Error: Interactor closed prematurely.", file=sys.stderr)
        return 1
    t1 = threading.Thread(target=forward_thread, args=("User", p_proc, l_proc), daemon=True)
    t2 = threading.Thread(target=forward_thread, args=("Judger", l_proc, p_proc), daemon=True)

    t1.start()
    t2.start()
    while True:
        ret1 = p_proc.poll()
        ret2 = l_proc.poll()
        
        if ret1 is not None or ret2 is not None:
            stop_flag = True
            time.sleep(0.05)
            break
        
        if stop_flag:
            break
            
        time.sleep(0.01)
    for proc in [p_proc, l_proc]:
        if proc.poll() is None:
            proc.terminate()
        proc.wait()
    
    sys.stderr.write(f"Exit Code: User={p_proc.returncode}, Judger={l_proc.returncode}\n")
    return l_proc.returncode

if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        sys.stderr.write("\nInterrupted.\n")
        sys.exit(1)

