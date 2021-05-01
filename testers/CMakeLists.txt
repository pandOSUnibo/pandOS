
if(PHASE STREQUAL "phase1")
    add_executable(tester p1test.c)
    target_link_options(tester PUBLIC -r )
    target_include_directories(tester PUBLIC ../h)
    target_link_libraries(tester phase1)
elseif(PHASE STREQUAL "phase2")
    add_library(tester p2test.c)
    target_link_options(tester PUBLIC -r )
    target_include_directories(tester PUBLIC ../h)
elseif(PHASE STREQUAL "phase3")
    # Compiles the flash devices
    add_custom_target(FLASH_DEVICES ALL cd ${CMAKE_CURRENT_SOURCE_DIR}/p3test && make)
endif()

