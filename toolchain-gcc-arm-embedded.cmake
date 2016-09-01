set(PREFIX "arm-none-eabi")

set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_VERSION    1)
set(CMAKE_SYSTEM_PROCESSOR  arm)

set(CMAKE_C_COMPILER ${PREFIX}-gcc CACHE INTERNAL "c compiler")
set(CMAKE_CXX_COMPILER ${PREFIX}-c++ CACHE INTERNAL "cxx compiler")
set(CMAKE_ASM_COMPILER ${PREFIX}-as CACHE INTERNAL "asm compiler")

set(CMAKE_OBJCOPY ${PREFIX}-objcopy CACHE INTERNAL "objcopy")
set(CMAKE_OBJDUMP ${PREFIX}-objdump CACHE INTERNAL "objdump")

set(CMAKE_AR ${PREFIX}-ar CACHE INTERNAL "archiver")

set(CMAKE_STRIP ${PREFIX}-strip CACHE INTERNAL "strip")
set(CMAKE_SIZE ${PREFIX}-size CACHE INTERNAL "size")

# Adjust the default behaviour of the FIND_XXX() commands:
# i)    Search headers and libraries in the target environment
# ii)   Search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Compilers like arm-none-eabi-gcc that target bare metal systems don't pass
# CMake's compiler check, so fill in the results manually and mark the test
# as passed:
set(CMAKE_COMPILER_IS_GNUCC     1)
set(CMAKE_C_COMPILER_ID         GNU)
set(CMAKE_C_COMPILER_ID_RUN     TRUE)
set(CMAKE_C_COMPILER_FORCED     TRUE)
set(CMAKE_CXX_COMPILER_ID       GNU)
set(CMAKE_CXX_COMPILER_ID_RUN   TRUE)
set(CMAKE_CXX_COMPILER_FORCED   TRUE)
