#!/usr/bin/env python3
"""Measure per-demo compilation time: C++17 vs C++20 with supplement libraries.

Usage:
    python3 scripts/compile_time.py [--iterations N] [--ftime-report] [--csv FILE]

Requires a configured host-debug build (cmake --preset host-debug).
"""

import argparse
import json
import os
import re
import shlex
import statistics
import subprocess
import sys
import tempfile
import time
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent
BUILD_DIR = PROJECT_ROOT / "build" / "host-debug"
COMPILE_COMMANDS = BUILD_DIR / "compile_commands.json"

# Map of (demo, version) -> substring that identifies the source file in
# compile_commands.json.  Demo 1 uses probe files in bench/.
FILE_MAP = {
    (1, 17): "bench/probe_demo1_cpp17.cpp",
    (1, 20): "bench/probe_demo1_cpp20.cpp",
    (2, 17): "demo2_ranges/cpp17/src/sensor_pipeline.cpp",
    (2, 20): "demo2_ranges/cpp20/src/sensor_pipeline.cpp",
    (3, 17): "demo3_expected/cpp17/src/sensor_init.cpp",
    (3, 20): "demo3_expected/cpp20/src/sensor_init.cpp",
    (4, 17): "demo4_consteval/cpp17/src/calibration_table.cpp",
    (4, 20): "demo4_consteval/cpp20/src/calibration_table.cpp",
}

DEMO_LABELS = {
    1: ("SFINAE traits", "concepts"),
    2: ("manual loops", "ranges-v3"),
    3: ("nested-if", "tl::expected"),
    4: ("runtime init", "consteval + GSL"),
}


def load_commands():
    if not COMPILE_COMMANDS.exists():
        sys.exit(f"error: {COMPILE_COMMANDS} not found.\n"
                 f"Run:  cmake --preset host-debug && cmake --build --preset host-debug")
    with open(COMPILE_COMMANDS) as f:
        return json.load(f)


def find_command(entries, file_substr):
    """Return (command_string, directory, output_file) for the matching entry."""
    for entry in entries:
        if file_substr in entry["file"]:
            cmd = entry.get("command") or " ".join(entry.get("arguments", []))
            return cmd, entry["directory"], entry.get("output", "")
    return None, None, None


def time_compilation(cmd_str, cwd, iterations, warmup=1):
    """Run the compilation command `iterations` times, return list of wall-clock seconds."""
    env = os.environ.copy()
    env["CCACHE_DISABLE"] = "1"

    args = shlex.split(cmd_str)

    # Find the -o flag to know which object file to delete between runs.
    obj_file = None
    for i, a in enumerate(args):
        if a == "-o" and i + 1 < len(args):
            obj_file = Path(cwd) / args[i + 1]
            break

    timings = []
    total = warmup + iterations
    for run in range(total):
        if obj_file and obj_file.exists():
            obj_file.unlink()
        start = time.perf_counter()
        result = subprocess.run(args, cwd=cwd, env=env,
                                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        elapsed = time.perf_counter() - start
        if result.returncode != 0:
            # Retry once showing stderr for diagnosis.
            result2 = subprocess.run(args, cwd=cwd, env=env,
                                     capture_output=True, text=True)
            sys.exit(f"Compilation failed (exit {result2.returncode}):\n"
                     f"  cmd: {cmd_str[:200]}\n"
                     f"  stderr: {result2.stderr[:500]}")
        if run >= warmup:
            timings.append(elapsed)
    return timings


def run_ftime_report(cmd_str, cwd):
    """Run once with -ftime-report appended; return parsed phase times."""
    env = os.environ.copy()
    env["CCACHE_DISABLE"] = "1"

    args = shlex.split(cmd_str)
    # Replace output with /dev/null to avoid clobbering real object.
    for i, a in enumerate(args):
        if a == "-o" and i + 1 < len(args):
            args[i + 1] = "/dev/null"
            break
    args.append("-ftime-report")
    result = subprocess.run(args, cwd=cwd, env=env, capture_output=True, text=True)
    if result.returncode != 0:
        return {}
    # Parse GCC -ftime-report output: lines like " phase parsing         :   0.12 (40%)  ..."
    phases = {}
    for line in result.stderr.splitlines():
        m = re.match(r'\s*(phase \S+.*?)\s*:\s*([\d.]+)\s*\(', line)
        if m:
            phases[m.group(1).strip()] = float(m.group(2))
    return phases


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--iterations", "-n", type=int, default=10,
                        help="number of timed iterations per file (default: 10)")
    parser.add_argument("--ftime-report", action="store_true",
                        help="run once with -ftime-report for phase breakdown")
    parser.add_argument("--csv", type=str, default=None,
                        help="write results to CSV file")
    args = parser.parse_args()

    entries = load_commands()
    results = {}  # (demo, version) -> {median, mean, stdev, min, max, timings}

    print(f"Measuring compilation times ({args.iterations} iterations, "
          f"{1} warmup run each)\n")

    for (demo, ver), file_sub in sorted(FILE_MAP.items()):
        cmd, cwd, _ = find_command(entries, file_sub)
        if cmd is None:
            print(f"  SKIP demo {demo} C++{ver}: not found in compile_commands.json")
            continue
        label = f"Demo {demo} C++{ver}"
        sys.stdout.write(f"  {label:20s} ... ")
        sys.stdout.flush()
        timings = time_compilation(cmd, cwd, args.iterations)
        med = statistics.median(timings)
        results[(demo, ver)] = {
            "median": med,
            "mean": statistics.mean(timings),
            "stdev": statistics.stdev(timings) if len(timings) > 1 else 0.0,
            "min": min(timings),
            "max": max(timings),
            "timings": timings,
        }
        print(f"{med*1000:7.0f} ms (median)")

    # Phase breakdown
    phase_data = {}
    if args.ftime_report:
        print("\nPhase breakdown (-ftime-report, single run):")
        for (demo, ver), file_sub in sorted(FILE_MAP.items()):
            cmd, cwd, _ = find_command(entries, file_sub)
            if cmd is None:
                continue
            phases = run_ftime_report(cmd, cwd)
            if phases:
                phase_data[(demo, ver)] = phases
                label = f"  Demo {demo} C++{ver}"
                print(f"{label}:")
                for phase, secs in sorted(phases.items(), key=lambda x: -x[1]):
                    print(f"    {phase:30s}  {secs*1000:7.1f} ms")

    # Summary table
    print("\n" + "=" * 90)
    print(f"{'Demo':<6} {'C++17':>30}  {'ms':>7}  {'C++20':>30}  {'ms':>7}  "
          f"{'Delta':>8}  {'Ratio':>6}")
    print("-" * 90)
    for demo in range(1, 5):
        r17 = results.get((demo, 17))
        r20 = results.get((demo, 20))
        if not r17 or not r20:
            continue
        l17, l20 = DEMO_LABELS[demo]
        m17, m20 = r17["median"] * 1000, r20["median"] * 1000
        delta = m20 - m17
        ratio = m20 / m17 if m17 > 0 else float("inf")
        print(f"  {demo:<4} {l17:>30}  {m17:7.0f}  {l20:>30}  {m20:7.0f}  "
              f"{delta:+7.0f}ms  {ratio:5.2f}x")
    print("=" * 90)

    # CSV output
    if args.csv:
        with open(args.csv, "w") as f:
            f.write("demo,version,label,median_ms,mean_ms,stdev_ms,min_ms,max_ms\n")
            for (demo, ver), r in sorted(results.items()):
                label = DEMO_LABELS[demo][0 if ver == 17 else 1]
                f.write(f"{demo},{ver},{label},"
                        f"{r['median']*1000:.1f},{r['mean']*1000:.1f},"
                        f"{r['stdev']*1000:.1f},{r['min']*1000:.1f},{r['max']*1000:.1f}\n")
        print(f"\nCSV written to {args.csv}")


if __name__ == "__main__":
    main()
