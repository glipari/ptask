# Add include directory
include_directories(.)
include_directories(../src/)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

add_executable(pbarr_example pbarr_example.c)
add_executable(modes modes.c)
add_executable(deadline_exception deadline_exception.c)
if(TRACE)
  add_executable(tracer_example tracer_example.c)
endif()

if(TRACE)
  target_link_libraries(tracer_example lttng ptask Threads::Threads m rt dl lttng-ust)
  target_link_libraries(pbarr_example lttng ptask Threads::Threads} m rt dl lttng-ust)
  target_link_libraries(modes lttng ptask Threads::Threads m rt dl lttng-ust)
  target_link_libraries(deadline_exception lttng ptask Threads::Threads m rt dl lttng-ust)
else()
  target_link_libraries(pbarr_example ptask Threads::Threads m rt)
  target_link_libraries(modes ptask Threads::Threads m rt)
  target_link_libraries(deadline_exception ptask Threads::Threads m rt)
endif()

add_subdirectory(fixed_prio)
add_subdirectory(edf)
