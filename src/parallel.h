#pragma once

#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int32_t *matrix_in;
    int32_t *matrix_out;

    size_t rows;
    size_t cols;
    size_t len;
    size_t radius;

    pthread_barrier_t barrier;
    pthread_mutex_t start_mutex;
    bool is_ready;
    size_t max_threads;

    size_t iterations;
} General;

typedef struct {
    General *general;

    size_t start;
    size_t end;

    int32_t *window;
} Task;

bool check(int error, const char *operation);

void *worker(void *arg);
