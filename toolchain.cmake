set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR umps)

set(XT_PRG_PREFIX mipsel-linux-gnu-)
set(CC ${XT_PRG_PREFIX}gcc)
set(LD ${XT_PRG_PREFIX}ld)
set(CMAKE_C_COMPILER  ${CC} CACHE STRING "compiler")
set(CMAKE_LINKER ${LD}  CACHE STRING "linker" )
#Data path
if(EXISTS "/usr/bin/umps3")
    set(UMPS3_DIR_PREFIX /usr)
else()
    set(UMPS3_DIR_PREFIX /usr/local)
endif()

set(UMPS3_DATA_DIR ${UMPS3_DIR_PREFIX}/share/umps3)
set(UMPS3_INCLUDE_DIR ${UMPS3_DIR_PREFIX}/include/umps3)

#Compiler options
set(CFLAGS_LANG -ffreestanding  )
set(CFLAGS_MIPS -mips1 -mabi=32 -mno-gpopt -G 0 -mno-abicalls -fno-pic -mfp32)
set(CFLAGS  ${CFLAGS_LANG} ${CFLAGS_MIPS} -I${UMPS3_INCLUDE_DIR} -Wall -O0)
string(REPLACE ";" " " CFLAGS_STRING "${CFLAGS}")
set(CMAKE_C_FLAGS ${CFLAGS_STRING}  CACHE STRING "flags of compiler")

# Linker options
set(LDFLAGS  -G 0 -nostdlib)
string(REPLACE ";" " " LDFLAGS_STRING "${LDFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS ${LDFLAGS_STRING}  CACHE STRING "linker flags")
