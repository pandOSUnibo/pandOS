add_executable(testers.o p1test.c)
target_link_options(testers.o PUBLIC -T ${UMPS3_DATA_DIR}/umpscore.ldscript ../crtso.o ../libumps.o)

target_include_directories(testers.o PUBLIC ../h)
target_link_libraries(testers.o phase1)
