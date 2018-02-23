# Add current directory.
include_directories(.)

# Create a library called "metasim" which includes the source files.
add_library(ptask "STATIC" pbarrier.c pmutex.c ptask.c ptime.c rtmode.c tstat.c libdl.c dle_timer.c calibrate.c)

# Export.
export(TARGETS ptask FILE "./ptaskConfig.cmake")
export(PACKAGE ptask)
