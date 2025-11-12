cmake_minimum_required(VERSION 3.16)
project(mini_matching_engine LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(matching
src/order_book.cpp
src/engine.cpp
)


target_include_directories(matching PUBLIC include)
target_compile_options(matching PRIVATE -O3 -march=native -Wall -Wextra -pthread)


add_executable(mini_match src/main.cpp)
target_link_libraries(mini_match PRIVATE matching pthread)


add_executable(benchmark bench/benchmark.cpp)
target_link_libraries(benchmark PRIVATE matching pthread)
