# Минимальная версия CMake.
cmake_minimum_required(VERSION 3.7.2)

# Проект.
project(AlFractal)

# Директория заголовочных файлов.
include_directories(include)

# Файлы исходного кода.
file(GLOB_RECURSE SOURCES "source/*.cpp") # - Automatically.

add_executable(AlFractal ${SOURCES}) # Using variable SOURCES.

# Флаги
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wpedantic -Wextra -fexceptions -O0 -g3 -ggdb --std=c++17")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wextra -O3 --std=c++17")

# Библиотеки
target_link_libraries(AlFractal m)
target_link_libraries(AlFractal gmp)
target_link_libraries(AlFractal gmpxx)
target_link_libraries(AlFractal pthread)
target_link_libraries(AlFractal sfml-system)
target_link_libraries(AlFractal sfml-graphics)
target_link_libraries(AlFractal sfml-audio)
target_link_libraries(AlFractal sfml-window)

