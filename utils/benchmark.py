import csv
from pathlib import Path
from statistics import median
import subprocess
from time import perf_counter


root = Path(__file__).resolve().parents[1]
program = root / "build" / "benchmark" / "median_filter"
output = root / "report" / "measurements_simple.csv"

configs = [
    {"file": "input_10x10.txt", "k": 5, "radius": 1},
    {"file": "input_20x50.txt", "k": 5, "radius": 1},
    {"file": "input_100x100.txt", "k": 5, "radius": 1},
    {"file": "input_200x500.txt", "k": 5, "radius": 1},
    {"file": "input_1000x1000.txt", "k": 5, "radius": 1},
]
thread_counts = [1, 2, 4, 8, 16]
repeats = 5

with output.open("w", encoding="utf-8-sig", newline="") as result_file:
    writer = csv.writer(result_file, delimiter=";")
    writer.writerow([
        "input_file", "rows", "cols", "k", "radius", "threads",
        "median_real_ms", "speedup", "efficiency_percent",
    ])

    for config in configs:
        text = (root / "data" / config["file"]).read_text()
        matrix = text.split("\n", 1)[1]
        rows, cols = map(int, matrix.split("\n", 1)[0].split())

        for threads in thread_counts:
            data = f"{config['k']} {config['radius']} {threads}\n" + matrix
            times = []

            for repeat in range(repeats):
                start = perf_counter()
                subprocess.run(
                    [str(program)], input=data, text=True, check=True,
                    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                )
                elapsed_ms = (perf_counter() - start) * 1000
                times.append(elapsed_ms)

            median_ms = median(times)
            if threads == 1:
                single_thread_ms = median_ms

            speedup = single_thread_ms / median_ms
            efficiency = 100 * speedup / threads
            row = [
                config["file"], rows, cols, config["k"], config["radius"], threads,
            ]
            for value in (median_ms, speedup, efficiency):
                row.append(f"{value:.6f}".replace(".", ","))
            writer.writerow(row)
            result_file.flush()
            print(f"{config['file']}, p={threads}: {median_ms:.3f} мс", flush=True)

print(f"Результаты: {output}")
