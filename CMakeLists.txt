cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE ./toolchain.cmake)
SET (CMAKE_C_COMPILER_WORKS 1)
project(kernel VERSION 0.1 LANGUAGES C)

# Option for generating the docs
option(COMPILE_DOC "Compile the documentation in pdf" OFF)
if(COMPILE_DOC)
    add_custom_target(DOC ALL 
        COMMAND cd ${PROJECT_SOURCE_DIR}/docs &&
        doxygen && 
        python3 make_source.py ../h ./source/ &&
        make latexpdf)
endif()

#Generate kernel
add_custom_target(${PROJECT_NAME} ALL
    COMMAND ${LD} -o ${PROJECT_NAME} ./testers/testers.o libumps.o crtso.o -T ${UMPS3_DATA_DIR}/umpscore.ldscript  ${LD_FLAGS})
#Generate kernel.core and kernel.stab
add_custom_target(kernel_umps ALL
    COMMAND umps3-elf2umps -k kernel)
#Compile umps3 library required
add_custom_target(libumps ALL
	COMMAND ${CC} ${CFLAGS} -I${UMPS3_INCLUDE_DIR} -c -o
        libumps.o ${UMPS3_DATA_DIR}/libumps.S)
        
add_custom_target(crtso ALL
    COMMAND ${CC} ${CFLAGS} -I${UMPS3_INCLUDE_DIR} -c -o
    crtso.o ${UMPS3_DATA_DIR}/crtso.S) 

#Add source folders
add_subdirectory(./testers)
add_subdirectory(./phase1)
#Add dependecies
add_dependencies(${PROJECT_NAME} testers.o)
add_dependencies(kernel_umps kernel)
