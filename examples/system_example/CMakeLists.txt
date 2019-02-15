# Add include directory
include_directories(.)
include_directories(../src/)
include_directories(${ALLEGRO_INCLUDE_DIR})

if(TRACE)
  add_executable(migrationTimeTest migrationTime.c)
  target_link_libraries(migrationTimeTest ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt dl lttng-ust)

  add_executable(schedulerTest testSystem.c graphics.c)
  target_link_libraries(schedulerTest ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt dl lttng-ust)

  add_executable(protocolTest testProt.c graphics.c)
  target_link_libraries(protocolTest ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt dl lttng-ust)

  add_executable(testPart testPart.c animation.c graphics.c)
  target_link_libraries(testPart ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt dl lttng-ust)
else()
  add_executable(migrationTimeTest migrationTime.c)
  target_link_libraries(migrationTimeTest ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt)

  add_executable(schedulerTest testSystem.c graphics.c)
  target_link_libraries(schedulerTest ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt)

  add_executable(protocolTest testProt.c graphics.c)
  target_link_libraries(protocolTest ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt)

  add_executable(testPart testPart.c animation.c graphics.c)
  target_link_libraries(testPart ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt)
endif()

file(COPY scheduler.dat DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY protocol.dat DESTINATION ${CMAKE_CURRENT_BINARY_DIR})