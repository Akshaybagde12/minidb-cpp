cmake_minimum_required(VERSION 3.16)
project(MiniDB VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(minidb_core
    src/bplustree.cpp
    src/storage.cpp
    src/database.cpp
    src/sql_parser.cpp
)
target_include_directories(minidb_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_options(minidb_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(minidb src/main.cpp)
target_link_libraries(minidb PRIVATE minidb_core)

add_executable(minidb_tests tests/basic_test.cpp)
target_link_libraries(minidb_tests PRIVATE minidb_core)

add_executable(minidb_benchmark benchmarks/benchmark.cpp)
target_link_libraries(minidb_benchmark PRIVATE minidb_core)

enable_testing()
add_test(NAME basic COMMAND minidb_tests)
