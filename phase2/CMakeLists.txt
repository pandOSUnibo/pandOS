if(PHASE STREQUAL "phase2")
    add_executable(phase2 initial.c exceptions.c interrupts.c scheduler.c syscalls.c)
    target_link_options(phase2 PUBLIC -r )

    target_include_directories(phase2 PUBLIC ../h)
    target_include_directories(phase2 PUBLIC ../testers/)

    target_link_libraries(phase2 phase1)
    target_link_libraries(phase2 tester)
endif()