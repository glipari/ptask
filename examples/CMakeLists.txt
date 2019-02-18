# Add include directory
include_directories(.)
include_directories(../src/)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

find_package(Allegro REQUIRED)

include_directories(${ALLEGRO_INCLUDE_DIR})

add_executable(ball ball.c)
add_executable(pbarr_example pbarr_example.c)
add_executable(modes modes.c)
add_executable(deadline_exception deadline_exception.c)
add_executable(pcp pcp.c)
if(TRACE)
  add_executable(tracer_example tracer_example.c)
endif()


if(TRACE)
  target_link_libraries(ball lttng ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt dl lttng-ust)
  target_link_libraries(tracer_example lttng ptask Threads::Threads m rt dl lttng-ust)
  target_link_libraries(pbarr_example lttng ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt dl lttng-ust)
  target_link_libraries(modes lttng ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt dl lttng-ust)
  target_link_libraries(deadline_exception lttng ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt dl lttng-ust)
  target_link_libraries(pcp lttng ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt dl lttng-ust)
else()
  target_link_libraries(ball ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt)
  target_link_libraries(pbarr_example ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt)
  target_link_libraries(modes ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt)
  target_link_libraries(deadline_exception ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt)
  target_link_libraries(pcp ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt)
endif()

file(COPY pcp.dat DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(activation_example)
add_subdirectory(advanced_act_test)
add_subdirectory(system_example)
add_subdirectory(fixed_prio)
