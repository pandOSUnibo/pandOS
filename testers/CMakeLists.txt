
if(PHASE STREQUAL "phase1")
    add_executable(tester p1test.c)
    target_link_options(tester PUBLIC -r )
    target_include_directories(tester PUBLIC ../h)
    target_link_libraries(tester phase1)
elseif(PHASE STREQUAL "phase2")
    add_library(tester p2test.c)
    target_link_options(tester PUBLIC -r )
    target_include_directories(tester PUBLIC ../h)
endif()

