import csv
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


folder = Path(__file__).resolve().parent
matrices = {}

with (folder / "measurements_simple.csv").open(encoding="utf-8-sig", newline="") as file:
    for row in csv.DictReader(file, delimiter=";"):
        size = f"{row['rows']}×{row['cols']}"
        if size not in matrices:
            matrices[size] = []
        matrices[size].append(row)

charts = [
    {
        "column": "speedup",
        "title": "Ускорение медианной фильтрации",
        "ylabel": "Ускорение",
        "file": "speedup.png",
    },
    {
        "column": "efficiency_percent",
        "title": "Эффективность медианной фильтрации",
        "ylabel": "Эффективность, %",
        "file": "efficiency.png",
    },
]

for chart in charts:
    plt.figure(figsize=(9, 5.5))
    for size, rows in matrices.items():
        rows = sorted(rows, key=lambda row: int(row["threads"]))
        threads = [int(row["threads"]) for row in rows]
        values = [float(row[chart["column"]].replace(",", ".")) for row in rows]
        plt.plot(threads, values, marker="o", label=size)

    plt.title(chart["title"])
    plt.xlabel("Количество потоков")
    plt.ylabel(chart["ylabel"])
    plt.xticks(threads)
    plt.ylim(bottom=0)
    plt.grid(alpha=0.3)
    plt.legend(title="Размер матрицы")
    plt.tight_layout()
    plt.savefig(folder / chart["file"], dpi=300)
    plt.close()
    print(f"Сохранён график: {folder / chart['file']}")
