# Set target platform
set(CMAKE_SYSTEM_NAME Linux)
# Set target architecture
set(CMAKE_SYSTEM_PROCESSOR armhf)

# We are not going to use this right now, but who knows?
#set(CMAKE_SYSROOT /path/to/rasp-pi-rootfs)

set(CMAKE_SYSTEM_VERSION 1)

set(UTIL_SEARCH_CMD which)

set(TOOLCHAIN_PREFIX arm-linux-gnueabihf-)

execute_process(
  COMMAND ${UTIL_SEARCH_CMD} ${TOOLCHAIN_PREFIX}gcc
  OUTPUT_VARIABLE BINUTILS_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${TOOLCHAIN_PREFIX}gcc -print-multiarch
  OUTPUT_VARIABLE GCC_TRIPLET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# WARNING: This will work only with ubuntu installed cross toolchains!!
# If you want to use downloaded toolchain, you will have to update
# variable ARM_GCC_ROOT_PATH with correct path
set(ARM_GCC_ROOT_PATH /usr/${GCC_TRIPLET})

# TODO: Maybe once it will work, need to look at this deeply
#execute_process(
#  COMMAND bash "-c" "${TOOLCHAIN_PREFIX}gcc -print-search-dirs |grep libraries | cut -d'=' -f2 | sed 's/:/ /g'"
#  OUTPUT_VARIABLE GCC_SEARCH_DIRS
#  OUTPUT_STRIP_TRAILING_WHITESPACE
#)

#get_filename_component(ARM_TOOLCHAIN_DIR ${BINUTILS_PATH} DIRECTORY)
# Without that flag CMake is not able to pass test compilation check
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)

#set(CMAKE_OBJCOPY ${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}objcopy CACHE INTERNAL "objcopy tool")
#set(CMAKE_SIZE_UTIL ${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}size CACHE INTERNAL "size tool")

# where is the target environment
set(CMAKE_FIND_ROOT_PATH ${ARM_GCC_ROOT_PATH})
#set(CMAKE_FIND_ROOT_PATH ${GCC_SEARCH_DIRS})
# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

