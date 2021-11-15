cmake_minimum_required(VERSION 3.1)

project(ptask)

SET(CMAKE_C_FLAGS "-Wall")
#SET (CMAKE_C_FLAGS "-Wall -fsanitize=address")
#SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory(src)
add_subdirectory(examples)

enable_testing()
add_subdirectory(tests)

