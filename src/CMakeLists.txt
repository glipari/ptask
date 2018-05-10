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
)

set(HEADER_FILES
  libdl.h
  pbarrier.h
  pmutex.h
  ptask.h
  ptime.h
  rtmode.h
  tstat.h
)

# Create a library called "ptask" which includes the source files.
add_library(ptask "STATIC" "${SOURCE_FILES}")
  
# Export.
export(TARGETS ptask FILE "./ptaskConfig.cmake")
export(PACKAGE ptask)
install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include)
