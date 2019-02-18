# Add include directory
include_directories(.)
include_directories(../src/)

add_executable(fixed_prio fixed.c)

if(TRACE)
  target_link_libraries(fixed_prio lttng ptask Threads::Threads m rt dl lttng-ust)
else()
  target_link_libraries(fixed_prio ptask Threads::Threads m rt)
endif()

file(COPY data.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

