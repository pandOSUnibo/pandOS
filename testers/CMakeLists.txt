add_executable(testers.o p2test.c)
target_link_options(testers.o PUBLIC -r )

target_include_directories(testers.o PUBLIC ../h)
target_link_libraries(testers.o phase1)
target_link_libraries(testers.o phase2)

