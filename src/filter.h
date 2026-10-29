#pragma once

#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

int32_t get_el(int32_t *matrix, size_t rows, size_t cols, int64_t row, int64_t col);

void fill_window(int32_t *matrix, size_t rows, size_t cols, int32_t *window, size_t radius, size_t row, size_t col);

int32_t *get_window(size_t radius);

int32_t get_median(int32_t *window, size_t radius);
