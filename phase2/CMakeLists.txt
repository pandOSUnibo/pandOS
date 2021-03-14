add_library(phase2 initial.c interrupts.c exceptions.c scheduler.c)
target_include_directories(phase2 PUBLIC ../h)
