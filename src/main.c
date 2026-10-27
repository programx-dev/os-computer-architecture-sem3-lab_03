#include <inttypes.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    int32_t *matrix_in;
    int32_t *matrix_out;

    size_t rows;
    size_t cols;
    size_t len;
    size_t radius;

    pthread_barrier_t barrier;

    size_t iterations;
} General;

typedef struct {
    General *general;

    size_t start;
    size_t end;

    int32_t *window;
} Task;

void print_matrix(int32_t *matrix, size_t rows, size_t cols) {
    size_t len = rows * cols;
    for (size_t i = 0; i < len; ++i) {
        printf("%" PRId32 " ", matrix[i]);
        if ((i + 1) % cols == 0) {
            printf("\n");
        }
    }
}

void print_arr(int32_t *arr, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%" PRId32 " ", arr[i]);
    }
    printf("\n");
}

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

bool check(int error, const char *operation) {
    if (error != 0) {
        fprintf(stderr, "%s: %s\n", operation, strerror(error));
        return false;
    }
    return true;
}

void *worker(void *arg) {
    Task *task = arg;

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

int main(void) {
    pthread_t *threads;

    Task *tasks;

    const size_t MAX_THREADS = 2; // Главный поток занимается только упралвением, здесь не учитывается

    int status = EXIT_SUCCESS;
    size_t i;
    size_t j;
    size_t created_threads = 0;

    General general;
    general.radius = 1;
    general.iterations = 1;
    int result;

    result = pthread_barrier_init(&general.barrier, NULL, MAX_THREADS);
    if (!check(result, "Create barrier")) {
        return EXIT_FAILURE;
    }

    FILE *file = fopen("../data/in.txt", "r");
    if (file == NULL) {
        perror("Can not open file.");
        return EXIT_FAILURE;
    }

    if (fscanf(file, "%zu %zu", &general.rows, &general.cols) != 2) {
        fprintf(stderr, "Error fscanf.");
        fclose(file);
        return EXIT_FAILURE;
    }

    general.len = general.rows * general.cols;

    general.matrix_in = malloc(sizeof(int32_t) * general.len);
    if (general.matrix_in == NULL) {
        perror("Can not allocate matrix.");
        fclose(file);
        return EXIT_FAILURE;
    }

    general.matrix_out = malloc(sizeof(int32_t) * general.len);
    if (general.matrix_out == NULL) {
        perror("Can not allocate matrix.");
        free(general.matrix_in);
        fclose(file);
        return EXIT_FAILURE;
    }

    threads = malloc(sizeof(*threads) * MAX_THREADS);
    if (threads == NULL) {
        perror("Can not allocate array.");
        free(general.matrix_in);
        free(general.matrix_out);
        fclose(file);
        return EXIT_FAILURE;
    }

    tasks = malloc(sizeof(*tasks) * MAX_THREADS);
    if (tasks == NULL) {
        perror("Can not allocate array.");
        fclose(file);
        free(threads);
        free(general.matrix_in);
        free(general.matrix_out);
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < general.len; ++i) {
        if (fscanf(file, "%" SCNd32, &general.matrix_in[i]) != 1) {
            fprintf(stderr, "Error fscanf.");
            fclose(file);
            free(general.matrix_in);
            free(general.matrix_out);
            free(threads);
            free(tasks);
            return EXIT_FAILURE;
        }
    }

    fclose(file);
    //
    size_t size_window = general.len / MAX_THREADS;

    size_t end_sum_windows = 0;
    for (size_t i = 0; i < MAX_THREADS; ++i) {
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
        tasks[i].end = end_sum_windows;

        tasks[i].general = &general;
    }

    tasks[MAX_THREADS - 1].end = general.len;
    // если size_window * MAX_THREADS < len_matrix ? - может ли такое быть или это улсовие можно убарть?

    for (i = 0; i < MAX_THREADS; ++i) {
        int error = pthread_create(&threads[i], NULL, worker, &tasks[i]);

        if (error != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(error));
            status = EXIT_FAILURE;
            break;
        }

        ++created_threads;
    }

    for (i = 0; i < created_threads; ++i) {
        int error = pthread_join(threads[i], NULL);

        if (error != 0) {
            fprintf(stderr, "pthread_join: %s\n", strerror(error));
            return EXIT_FAILURE;
        }
    }

    if (general.iterations % 2 == 0) {
        int32_t *tmp = general.matrix_in;
        general.matrix_in = general.matrix_out;
        general.matrix_out = tmp;
    }

    if (status == EXIT_SUCCESS) {
        print_matrix(general.matrix_out, general.rows, general.cols);
    }

    //
    for (i = 0; i < MAX_THREADS; ++i) {
        free(tasks[i].window);
    }

    free(tasks);
    free(threads);
    free(general.matrix_in);
    free(general.matrix_out);

    return status;
}
