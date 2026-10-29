#!/usr/bin/env python3
"""Восстановление изображения в оттенках серого из результата C-программы."""

import sys
from pathlib import Path

from PIL import Image


DATA_DIR = Path(__file__).resolve().parents[1] / "data"
MATRIX_PATH = DATA_DIR / "out.txt"
IMAGE_PATH = DATA_DIR / "image_out.png"


def matrix_to_image(result_path: Path, image_path: Path) -> None:
    if image_path.resolve() == result_path.resolve():
        raise ValueError("Изображение нельзя сохранять поверх файла матрицы.")

    # Размеры и значения читаются из одного выходного файла C-программы.
    with result_path.open(encoding="utf-8-sig") as result:
        shape = result.readline().split()
        text = result.read()

    if len(shape) != 2:
        raise ValueError("Первая строка результата должна содержать rows cols.")

    rows, cols = (int(value) for value in shape)
    if rows <= 0 or cols <= 0:
        raise ValueError("Количество строк и столбцов должно быть положительным.")

    try:
        values = [int(value) for value in text.split()]
    except ValueError as error:
        raise ValueError("После строки размеров ожидаются только целые числа.") from error

    expected = rows * cols
    if len(values) != expected:
        raise ValueError(f"Ожидалось {expected} чисел, получено {len(values)}.")
    if any(value < 0 or value > 255 for value in values):
        raise ValueError("Значения яркости должны находиться в диапазоне 0..255.")

    # В режиме L одно число задаёт яркость одного пикселя.
    image = Image.frombytes("L", (cols, rows), bytes(values))
    image.save(image_path, format="PNG")
    print(f"Сохранено изображение {cols} x {rows}: {image_path}")


def main() -> None:
    try:
        matrix_to_image(MATRIX_PATH, IMAGE_PATH)
    except (OSError, ValueError) as error:
        print(f"Ошибка: {error}", file=sys.stderr)
        raise SystemExit(1)


if __name__ == "__main__":
    main()
