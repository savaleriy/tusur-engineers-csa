cmake_minimum_required(VERSION 3.16)
project(course_project LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(core STATIC src/core.c)
target_include_directories(core PUBLIC include)
target_compile_options(core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(app src/main.c)
target_link_libraries(app PRIVATE core)
target_compile_options(app PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_executable(test_core tests/test_core.c)
target_link_libraries(test_core PRIVATE core)
target_compile_options(test_core PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME core COMMAND test_core)
