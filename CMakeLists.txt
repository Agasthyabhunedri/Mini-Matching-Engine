cmake_minimum_required(VERSION 3.20)
project(MiniMatchingEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable higher optimization for Release
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_library(mmx STATIC
    src/MatchingEngine.cpp
)
target_include_directories(mmx PUBLIC include)
target_compile_options(mmx PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/O2 /DNOMINMAX /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-O3 -march=native -mtune=native -fno-exceptions -fno-rtti>
)

add_executable(mmx_bench
    src/main.cpp
)
target_link_libraries(mmx_bench PRIVATE mmx)
target_compile_options(mmx_bench PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/O2 /DNOMINMAX /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-O3 -march=native -mtune=native -fno-exceptions -fno-rtti>
)

add_executable(mmx_tests
    tests/basic.cpp
)
target_link_libraries(mmx_tests PRIVATE mmx)
