# Add include directory
include_directories(.)
include_directories(../src/)
include_directories(${ALLEGRO_INCLUDE_DIR})

add_executable(activation_example animation.c graphics.c testTaskFun.c)

if(TRACE)
  target_link_libraries(activation_example ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt dl lttng-ust)
else()
  target_link_libraries(activation_example ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt)
endif()
