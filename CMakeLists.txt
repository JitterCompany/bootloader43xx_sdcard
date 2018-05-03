cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)

set(CMAKE_TOOLCHAIN_FILE    ${CMAKE_SOURCE_DIR}/cmake/toolchain-gcc-arm-embedded.cmake)

project(bootloader)

#------------------------------------------------------------------------------
# Required CPM Setup - no need to modify - See: https://github.com/iauns/cpm
#------------------------------------------------------------------------------
set(CPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpm_packages" CACHE TYPE STRING)
find_package(Git)
if(NOT GIT_FOUND)
    message(FATAL_ERROR "CPM requires Git.")
endif()
if (NOT EXISTS ${CPM_DIR}/CPM.cmake)
    message(STATUS "Cloning repo (https://github.com/iauns/cpm)")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" clone https://github.com/iauns/cpm ${CPM_DIR}
        RESULT_VARIABLE error_code
        OUTPUT_QUIET ERROR_QUIET)
    if(error_code)
        message(FATAL_ERROR "CPM failed to get the hash for HEAD")
    endif()
endif()
include(${CPM_DIR}/CPM.cmake)


#------------------------------------------------------------------------------
# Detect Board Specific Settings
#------------------------------------------------------------------------------

# defaults
set(BOARD               "unknown")
set(OPTIMIZE            0)

# find board config
include(${CMAKE_SOURCE_DIR}/board_config.cmake)

add_definitions(-DOPTIMIZE=${OPTIMIZE})
add_definitions(-BOARD=${BOARD})

# build options string: only append non-default options


string(REPLACE "BOARD_" "" BOARD_ID "${BOARD}")
string(REPLACE "-" "_" BOARD_STR "${BOARD_ID}")
list(APPEND BUILD_OPTS "-${BOARD_STR}")

if(NOT("${OPTIMIZE}" STREQUAL "2"))
    list(APPEND BUILD_OPTS "-O${OPTIMIZE}")
endif()

string(REPLACE ";" "" BUILD_OPTS "${BUILD_OPTS}")


#-----------------------------------------------------------------------
# Build settings
#-----------------------------------------------------------------------

set(EXE_NAME                bootloader)
set(FLASH_ADDR              0x1A000000)
set(FLASH_CFG               lpc4337_swd)
set(DEBUG_BREAKPOINT_LIMIT  6)
set(DEBUG_WATCHPOINT_LIMIT  4)

set(CMAKE_GDB           arm-none-eabi-gdb-py)

set(OPT 0)

set(SYSTEM_LIBRARIES    m c gcc)

set(FLAGS_M4 "-mcpu=cortex-m4")

set(C_FLAGS "-O${OPT} -g3 -c -fmessage-length=80 -fno-builtin   \
    -ffunction-sections -fdata-sections -std=gnu99 -mthumb      \
    -fdiagnostics-color=auto")
set(C_FLAGS_WARN "-Wall -Wextra -Wno-unused-parameter           \
    -Wshadow -Wpointer-arith -Winit-self -Wstrict-overflow=5")

set(L_FLAGS "-fmessage-length=80 -nostdlib -specs=nano.specs \
    -mthumb -Wl,--gc-sections")

set(MCU_PLATFORM    43xx_m4)

set(C_FLAGS_PLATFORM    "-DMCU_PLATFORM_${MCU_PLATFORM}")

add_definitions("${FLAGS_M4} ${C_FLAGS} ${C_FLAGS_WARN} ${C_FLAGS_PLATFORM}")
add_definitions(-DCORE_M4)

# fatfs_lib overrides
add_definitions(-D_FS_READONLY=1)
add_definitions(-D_FS_MINIMIZE=2)
add_definitions(-DFF_FS_NORTC=1)


set(ELF_PATH            "${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}")
set(EXE_PATH            "${ELF_PATH}.bin")
set(FLASH_FILE          ${PROJECT_BINARY_DIR}/flash.cfg)

#------------------------------------------------------------------------------
# CPM Modules
#------------------------------------------------------------------------------

CPM_AddModule("43xx_m4_startup"
    GIT_REPOSITORY "https://github.com/JitterCompany/43xx_m4_startup.git"
    GIT_TAG "1.1")

CPM_AddModule("c_utils"
    GIT_REPOSITORY "https://github.com/JitterCompany/c_utils"
    GIT_TAG "1.4.5")

CPM_AddModule("lpc_tools"
    GIT_REPOSITORY "https://github.com/JitterCompany/lpc_tools"
    GIT_TAG "2.5.1")

CPM_AddModule("fatfs_lib"
    GIT_REPOSITORY "https://github.com/JitterCompany/fatfs_lib"
    GIT_TAG "1.2")

CPM_AddModule("mcu_timing"
    GIT_REPOSITORY "https://github.com/JitterCompany/mcu_timing"
    GIT_TAG "1.5.4")

CPM_AddModule("mcu_sdcard"
    GIT_REPOSITORY "https://github.com/JitterCompany/mcu_sdcard"
    GIT_TAG "0.3")

CPM_AddModule("openocd_tools"
    GIT_REPOSITORY "https://github.com/JitterCompany/openocd_tools.git"
    GIT_TAG "1.4")

CPM_AddModule("lpc_chip_43xx_m4"
    GIT_REPOSITORY "https://github.com/JitterCompany/lpc_chip_43xx_m4.git"
    GIT_TAG "3.1.2")


# mbed_tls: use our own src/mbedtls_cfg.h instead of the default
set(TLS_CFG_INCLUDE "<${CMAKE_CURRENT_SOURCE_DIR}/src/mbedtls_cfg.h>")
add_definitions("-DMBEDTLS_CONFIG_FILE=${TLS_CFG_INCLUDE}")

CPM_AddModule("cpm_mbedtls"
    GIT_REPOSITORY "https://github.com/JitterCompany/cpm_mbedtls.git"
GIT_TAG "1.1")

CPM_Finish()


get_property(startup_linker GLOBAL PROPERTY startup_linker)
message(STATUS "bootloader: startup_linker: ${startup_linker}")

set(LINKER_FILES "-L .. -T ${startup_linker}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${L_FLAGS} \
${LINKER_FILES} ${FLAGS_M4}")


#-----------------------------------------------------------------------
# Setup source
#-----------------------------------------------------------------------

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/src/board_config/${BOARD_ID}/")
    message(FATAL_ERROR "Board '${BOARD_ID}' not supported!\nPlease check board_config.cmake!")
endif()

include_directories("${CMAKE_SOURCE_DIR}/src/")

file(GLOB SOURCES 
"src/*.c"
"src/board_config/${BOARD_ID}/*.c"
)

set(CMAKE_SYSTEM_NAME Generic)

#-----------------------------------------------------------------------
# Setup executable
#-----------------------------------------------------------------------

set(FW_VERSION_FILE "${CMAKE_SOURCE_DIR}/src/generated/bootloader_version.h")

add_custom_command(OUTPUT ${FW_VERSION_FILE}
    DEPENDS version
    )

add_custom_target(version
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/version.cmake "${FW_VERSION_FILE}" "${BUILD_OPTS}" 
    )


add_executable(${EXE_NAME} ${SOURCES} ${FW_VERSION_FILE})
target_link_libraries(${EXE_NAME} ${CPM_LIBRARIES})
target_link_libraries(${EXE_NAME} ${SYSTEM_LIBRARIES})


add_custom_target(bin ALL
    # empty flash file
    COMMAND > "${FLASH_FILE}"

    DEPENDS ${EXE_NAME}
    COMMAND ${CMAKE_OBJCOPY} -O binary ${EXE_NAME} ${EXE_NAME}.bin

    # append flash file
    COMMAND echo "${PROJECT_BINARY_DIR}/${EXE_NAME}.bin ${FLASH_ADDR} ${FLASH_CFG} ${MCU_PLATFORM}" >> "${PROJECT_BINARY_DIR}/flash.cfg"
    )

add_dependencies(flash bin)
