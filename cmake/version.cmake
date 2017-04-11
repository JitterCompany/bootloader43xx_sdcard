
# this header will be generated
set(FW_VERSION_FILE "${CMAKE_ARGV3}")
set(BUILD_OPTS "${CMAKE_ARGV4}")

# find firmware version
execute_process(OUTPUT_VARIABLE FIRMWARE_VERSION
    COMMAND git describe --long --tags --dirty --always
    COMMAND tr -d "\n"
    )
file(WRITE "${FW_VERSION_FILE}" "#define BOOTLOADER_VERSION \"${FIRMWARE_VERSION}${BUILD_OPTS}\"\n")
message(STATUS "${EXE_NAME}: Detected firmware version '${FIRMWARE_VERSION}'")
message(STATUS "${EXE_NAME}: Detected build options '${BUILD_OPTS}'")

