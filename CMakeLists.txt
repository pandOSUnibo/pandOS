cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE ./toolchain.cmake)
SET (CMAKE_C_COMPILER_WORKS 1)
project(kernel VERSION 0.1 LANGUAGES C)


add_custom_target(${PROJECT_NAME} ALL
    COMMAND ${LD} -o ${PROJECT_NAME} ./testers/testers.o ${LD_FLAGS})

add_custom_target(kernel_umps ALL
    COMMAND umps3-elf2umps -k kernel)

add_custom_target(libumps ALL
	COMMAND ${CC} ${CFLAGS} -I${UMPS3_INCLUDE_DIR} -c -o
        libumps.o ${UMPS3_DATA_DIR}/libumps.S)
        
add_custom_target(crtso ALL
    COMMAND ${CC} ${CFLAGS} -I${UMPS3_INCLUDE_DIR} -c -o
    crtso.o ${UMPS3_DATA_DIR}/crtso.S) 

add_subdirectory(./testers)
add_subdirectory(./phase1)

add_dependencies(${PROJECT_NAME} testers.o)
add_dependencies(kernel_umps kernel)
