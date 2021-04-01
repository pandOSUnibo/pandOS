cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE ./toolchain.cmake)
project(kernel VERSION 0.2.0 LANGUAGES C)

# Option for generating the docs
option(COMPILE_DOC "Compile the documentation in pdf" OFF)
set(PHASE CACHE STRING "Phase to compile") 

if(COMPILE_DOC)
    add_custom_target(DOC ALL 
        COMMAND cd ${PROJECT_SOURCE_DIR}/docs &&
        doxygen && 
        python3 make_source.py ../h ./source/ &&
        make latexpdf)
endif()

#Checks what phase to compile
if(PHASE STREQUAL "phase1")
    set(TARGET_PATH ./testers/)
    set(TARGET tester)
elseif(PHASE STREQUAL "phase2")
    set(TARGET_PATH ./phase2/)
    set(TARGET phase2)
else()
    message(FATAL_ERROR "Phase must be equal to 'phase1' or 'phase2'." )    
endif()

#Generate kernel
add_custom_target(${PROJECT_NAME} ALL
    COMMAND ${LD} -o ${PROJECT_NAME} ${TARGET_PATH}${TARGET} libumps.o crtso.o -T ${UMPS3_DATA_DIR}/umpscore.ldscript  ${LD_FLAGS})

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
add_subdirectory(./phase2)

#Add dependecies
add_dependencies(${PROJECT_NAME} ${TARGET} libumps crtso)
add_dependencies(kernel_umps kernel)
