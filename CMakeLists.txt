cmake_minimum_required(VERSION 3.20)

project(lab_03_variant_11 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(median_filter
    src/main.c
    # Добавить src/filter.c и src/paralell.c, когда в них появится код.
)

target_compile_options(median_filter PRIVATE -Wall -Wextra -Werror -pedantic)
target_compile_definitions(median_filter PRIVATE _GNU_SOURCE)
target_link_libraries(median_filter PRIVATE Threads::Threads)
