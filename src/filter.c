#include "filter.h"
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

int32_t get_el(int32_t *matrix, size_t rows, size_t cols, int64_t row, int64_t col) {
    if (row < 0) {
        row = 0;
    } else if (row >= (int64_t)rows) {
        row = (int64_t)rows - 1;
    }

    if (col < 0) {
        col = 0;
    } else if (col >= (int64_t)cols) {
        col = (int64_t)cols - 1;
    }

    return matrix[(size_t)row * cols + (size_t)col];
}

void fill_window(int32_t *matrix, size_t rows, size_t cols, int32_t *window, size_t radius, size_t row, size_t col) {
    size_t l = 0;
    for (int64_t i = (int64_t)row - (int64_t)radius; i < (int64_t)row + (int64_t)radius + 1; ++i) {
        for (int64_t j = (int64_t)col - (int64_t)radius; j < (int64_t)col + (int64_t)radius + 1; ++j) {
            window[l++] = get_el(matrix, rows, cols, i, j);
        }
    }
}

int32_t *get_window(size_t radius) {
    size_t count = (2 * radius + 1) * (2 * radius + 1);

    return malloc(count * sizeof(int32_t));
}

int compare_values(const void *left, const void *right) {
    int32_t a = *(const int32_t *)left;
    int32_t b = *(const int32_t *)right;

    return (a > b) - (a < b);
}

int32_t get_median(int32_t *window, size_t radius) {
    size_t count = (2 * radius + 1) * (2 * radius + 1);
    qsort(window, count, sizeof(int32_t), compare_values);

    return window[count / 2];
}
