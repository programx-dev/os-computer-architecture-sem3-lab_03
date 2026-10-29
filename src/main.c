#include "filter.h"
#include "parallel.h"
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool write_matrix(FILE *file, const int32_t *matrix, size_t rows, size_t cols) {
    size_t len = rows * cols;

    if (fprintf(file, "%zu %zu\n", rows, cols) < 0) {
        return false;
    }

    for (size_t i = 0; i < len; ++i) {
        if (fprintf(file, "%" PRId32 " ", matrix[i]) < 0) {
            return false;
        }
        if ((i + 1) % cols == 0) {
            if (fprintf(file, "\n") < 0) {
                return false;
            }
        }
    }

    return true;
}

void print_arr(int32_t *arr, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%" PRId32 " ", arr[i]);
    }
    printf("\n");
}

int main(void) {
    Task *tasks;
    General general;

    int status = EXIT_SUCCESS;
    size_t i;
    size_t j;

    pthread_t *threads;
    size_t created_threads = 0;

    general.is_ready = false;
    int iterations;
    int radius;
    int max_threads;

    // Ввод
    fprintf(stderr, "main: PID=%d TID=%d\n", getpid(), gettid());
    fprintf(stderr, "iterations radius max_threads: ");

    if (scanf("%d %d %d", &iterations, &radius, &max_threads) != 3) {
        fprintf(stderr, "Error scanf.");
        return EXIT_FAILURE;
    }

    if (iterations <= 0) {
        fprintf(stderr, "iterations expected > 0.\n");
        return EXIT_FAILURE;
    }

    if (radius <= 0) {
        fprintf(stderr, "radius expected > 0.\n");
        return EXIT_FAILURE;
    }

    if (max_threads <= 0) {
        fprintf(stderr, "max_threads expected > 0.\n");
        return EXIT_FAILURE;
    }

    general.iterations = iterations;
    general.radius = radius;
    general.max_threads = max_threads;

    fprintf(stderr, "rows cols: ");
    if (scanf("%zu %zu", &general.rows, &general.cols) != 2) {
        fprintf(stderr, "Error scanf.");
        return EXIT_FAILURE;
    }

    if (general.rows == 0 || general.cols == 0) {
        fprintf(stderr, "rows and cols expected > 0.\n");
        return EXIT_FAILURE;
    }

    general.len = general.rows * general.cols;

    general.matrix_in = malloc(sizeof(int32_t) * general.len);
    if (general.matrix_in == NULL) {
        perror("Can not allocate matrix.");
        return EXIT_FAILURE;
    }

    general.matrix_out = malloc(sizeof(int32_t) * general.len);
    if (general.matrix_out == NULL) {
        perror("Can not allocate matrix.");
        free(general.matrix_in);
        return EXIT_FAILURE;
    }

    threads = malloc(sizeof(*threads) * general.max_threads);
    if (threads == NULL) {
        perror("Can not allocate array.");
        free(general.matrix_in);
        free(general.matrix_out);
        return EXIT_FAILURE;
    }

    tasks = malloc(sizeof(*tasks) * general.max_threads);
    if (tasks == NULL) {
        perror("Can not allocate array.");
        free(threads);
        free(general.matrix_in);
        free(general.matrix_out);
        return EXIT_FAILURE;
    }

    fprintf(stderr, "matrix:\n");
    for (i = 0; i < general.len; ++i) {
        if (scanf("%" SCNd32, &general.matrix_in[i]) != 1) {
            fprintf(stderr, "Error scanf.");
            free(general.matrix_in);
            free(general.matrix_out);
            free(threads);
            free(tasks);
            return EXIT_FAILURE;
        }
    }

    // Создание задач
    size_t size_window = general.len / general.max_threads;
    size_t extra = general.len % general.max_threads;
    size_t end_sum_windows = 0;
    for (i = 0; i < general.max_threads; ++i) {
        int32_t *window = get_window(general.radius);

        if (window == NULL) {
            perror("Error allocate window");

            for (j = 0; j < i; ++j) {
                free(tasks[j].window);
            }

            free(general.matrix_in);
            free(general.matrix_out);
            free(tasks);
            free(threads);
            return EXIT_FAILURE;
        }

        tasks[i].window = window;
        tasks[i].start = end_sum_windows;
        end_sum_windows += size_window;
        if (i < extra) {
            ++end_sum_windows;
        }
        tasks[i].end = end_sum_windows;

        tasks[i].general = &general;
    }

    // Создание барьера и мютекса
    if (!check(pthread_barrier_init(&general.barrier, NULL, general.max_threads), "Create barrier")) {
        for (i = 0; i < general.max_threads; ++i) {
            free(tasks[i].window);
        }
        free(tasks);
        free(threads);
        free(general.matrix_in);
        free(general.matrix_out);
        return EXIT_FAILURE;
    }

    if (!check(pthread_mutex_init(&general.start_mutex, NULL), "pthread_mutex_init")) {
        check(pthread_barrier_destroy(&general.barrier), "pthread_barrier_destroy");
        for (i = 0; i < general.max_threads; ++i) {
            free(tasks[i].window);
        }
        free(tasks);
        free(threads);
        free(general.matrix_in);
        free(general.matrix_out);
        return EXIT_FAILURE;
    }

    if (!check(pthread_mutex_lock(&general.start_mutex), "pthread_mutex_lock")) {
        check(pthread_mutex_destroy(&general.start_mutex), "pthread_mutex_destroy");
        check(pthread_barrier_destroy(&general.barrier), "pthread_barrier_destroy");
        for (i = 0; i < general.max_threads; ++i) {
            free(tasks[i].window);
        }
        free(tasks);
        free(threads);
        free(general.matrix_in);
        free(general.matrix_out);
        return EXIT_FAILURE;
    }

    // Создание потоков
    for (i = 1; i < general.max_threads; ++i) {
        int error = pthread_create(&threads[i - 1], NULL, worker, &tasks[i]);

        if (error != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(error));
            status = EXIT_FAILURE;
            break;
        }

        ++created_threads;
    }

    if (created_threads == general.max_threads - 1) {
        general.is_ready = true;
    }

    if (!check(pthread_mutex_unlock(&general.start_mutex), "pthread_mutex_unlock")) {
        return EXIT_FAILURE;
    }

    if (status == EXIT_SUCCESS) {
        worker(&tasks[0]);
    }

    // Ожидание завершения
    for (i = 0; i < created_threads; ++i) {
        int error = pthread_join(threads[i], NULL);

        if (error != 0) {
            fprintf(stderr, "pthread_join: %s\n", strerror(error));
            return EXIT_FAILURE;
        }
    }

    if (!check(pthread_barrier_destroy(&general.barrier), "pthread_barrier_destroy")) {
        status = EXIT_FAILURE;
    }

    if (!check(pthread_mutex_destroy(&general.start_mutex), "pthread_mutex_destroy")) {
        status = EXIT_FAILURE;
    }

    // Вывод
    if (general.iterations % 2 == 0) {
        int32_t *tmp = general.matrix_in;
        general.matrix_in = general.matrix_out;
        general.matrix_out = tmp;
    }

    if (status == EXIT_SUCCESS) {
        if (!write_matrix(stdout, general.matrix_out, general.rows, general.cols)) {
            perror("write_matrix");
            status = EXIT_FAILURE;
        }
    }

    // Очистка
    for (i = 0; i < general.max_threads; ++i) {
        free(tasks[i].window);
    }

    free(tasks);
    free(threads);
    free(general.matrix_in);
    free(general.matrix_out);

    return status;
}
