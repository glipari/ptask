cmake_minimum_required(VERSION 3.1)

project(ptask)

SET(CMAKE_C_FLAGS "-Wall")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory(src)
add_subdirectory(examples)

enable_testing()
add_subdirectory(tests)
