#include "parallel.h"
#include "filter.h"
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool check(int error, const char *operation) {
    if (error != 0) {
        fprintf(stderr, "%s: %s\n", operation, strerror(error));
        return false;
    }
    return true;
}

void *worker(void *arg) {
    Task *task = arg;

    //
    if (!check(pthread_mutex_lock(&task->general->start_mutex), "pthread_mutex_lock")) {
        exit(EXIT_FAILURE);
    }

    bool is_ready = task->general->is_ready;

    if (!check(pthread_mutex_unlock(&task->general->start_mutex), "pthread_mutex_unlock")) {
        exit(EXIT_FAILURE);
    }

    if (!is_ready) {
        return NULL;
    }

    fprintf(stderr, "worker: PID=%d TID=%d range=[%zu, %zu)\n",
            getpid(), gettid(), task->start, task->end);
    //

    pthread_barrier_t *barrier = &task->general->barrier;
    size_t start = task->start;
    size_t end = task->end;
    int32_t *window = task->window;
    int32_t *matrix_in = task->general->matrix_in;
    int32_t *matrix_out = task->general->matrix_out;
    size_t rows = task->general->rows;
    size_t cols = task->general->cols;
    size_t radius = task->general->radius;
    size_t iterations = task->general->iterations;

    size_t i;
    size_t k;

    for (k = 0; k < iterations; ++k) {
        for (i = start; i < end; ++i) {
            fill_window(matrix_in, rows, cols, window, radius, i / cols, i % cols);
            matrix_out[i] = get_median(window, radius);
        }

        int result = pthread_barrier_wait(barrier);
        if (result != PTHREAD_BARRIER_SERIAL_THREAD) {
            if (!check(result, "pthread_barrier_wait")) {
                exit(EXIT_FAILURE);
            }
        }

        int32_t *tmp = matrix_in;
        matrix_in = matrix_out;
        matrix_out = tmp;
    }

    return NULL;
}
