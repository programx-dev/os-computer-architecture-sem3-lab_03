#!/usr/bin/env python3
"""Преобразование изображения в целочисленную матрицу яркости для лабораторной."""

import sys
from pathlib import Path

from PIL import Image


DATA_DIR = Path(__file__).resolve().parents[1] / "data"
IMAGE_PATH = DATA_DIR / "image.png"
MATRIX_PATH = DATA_DIR / "in.txt"


def image_to_matrix(image_path: Path, matrix_path: Path) -> None:
    if image_path.resolve() == matrix_path.resolve():
        raise ValueError("Изображение и матрица должны быть разными файлами.")

    with Image.open(image_path) as source:
        image = source.convert("L")
        width, height = image.size
        pixels = image.tobytes()

    # Перевод в оттенки серого выполняется в памяти: одно число на пиксель.
    cols = width
    with matrix_path.open("w", encoding="utf-8") as matrix:
        matrix.write(f"{height} {cols}\n")
        for row in range(height):
            start = row * cols
            values = pixels[start:start + cols]
            matrix.write(" ".join(str(value) for value in values) + "\n")

    print(f"Изображение: {width} x {height} пикселей.")
    print(f"Матрица: {height} строк, {cols} столбцов.")
    print(f"Сохранено: {matrix_path}")


def main() -> None:
    try:
        image_to_matrix(IMAGE_PATH, MATRIX_PATH)
    except (OSError, ValueError) as error:
        print(f"Ошибка: {error}", file=sys.stderr)
        raise SystemExit(1)


if __name__ == "__main__":
    main()
