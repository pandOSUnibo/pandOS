if(PHASE STREQUAL "phase3")
    add_executable(phase3 init_proc.c sys_support.c vm_support.c)
    target_link_options(phase3 PUBLIC -r )

    target_include_directories(phase3 PUBLIC ../h)

    target_link_libraries(phase2 phase1)
    target_link_libraries(phase3 phase2)
endif()