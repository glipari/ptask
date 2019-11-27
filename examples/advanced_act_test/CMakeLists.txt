# Add include directory
include_directories(.)
include_directories(../src/)
include_directories(${ALLEGRO_INCLUDE_DIR})

add_executable(advanced_act_test animation.c graphics.c testParam.c)
if(TRACE)
  target_link_libraries(advanced_act_test ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt dl lttng-ust)
else()
  target_link_libraries(advanced_act_test ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt)
endif()

file(COPY param.dat DESTINATION ${CMAKE_CURRENT_BINARY_DIR})