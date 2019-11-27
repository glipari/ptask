# Add current directory.
include_directories(.)

set(SOURCE_FILES
  pbarrier.c
  pmutex.c
  ptask.c
  ptime.c
  rtmode.c
  tstat.c
  libdl.c
  dle_timer.c
  calibrate.c
)

set(HEADER_FILES
  libdl.h
  pbarrier.h
  pmutex.h
  ptask.h
  ptime.h
  rtmode.h
  tstat.h
  dle_timer.h
  calibrate.h
  if(TRACE)
    tpt_provider.h
  endif()
)

# Create a library called "ptask" which includes the source files.
add_library(ptask "STATIC" "${SOURCE_FILES}")

if(TRACE)
  target_compile_options(ptask "PRIVATE" "-DTRACEPOINT_DEFINE")
  add_library(lttng "STATIC" "tpt_provider.c")
  target_compile_options(lttng "PRIVATE" "-I.")
endif()
  
# Export.
export(TARGETS ptask FILE "./ptaskConfig.cmake")
export(PACKAGE ptask)

if(TRACE)
  export(TARGETS lttng FILE "./ptaskConfig.cmake")
  export(PACKAGE lttng) #mie
endif()

install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include)
