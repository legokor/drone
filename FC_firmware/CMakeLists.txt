cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME FC_firmware)
set(CMAKE_BINARY_DIR build/)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx build/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Drivers/bar/impl/bar.c
    Drivers/esc/impl/esc.c
    Drivers/gps/impl/gps.c
    Drivers/imu/impl/imu.c
    Drivers/lora/impl/lora.c
    Drivers/mag/impl/mag.c
    Drivers/rc/impl/rc.c
    Drivers/sd/impl/sd.c
    Drivers/uart/impl/uart.c
    Drivers/usb/impl/usb.c

    Src/act/impl/act.c
    Src/ctrl/impl/ctrl.c
    Src/dsp/impl/dsp.c
    Src/err/impl/err.c
    Src/guide/impl/guide.c
    Src/irq/impl/irq_manager.c
    Src/llc/impl/llc.c
    Src/log/impl/log.c
    Src/sys/impl/sys.c
    Src/tel/impl/tel.c
    Src/utils/impl/utils.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    Drivers
    Src
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
)

# CMSIS DSP
include(FetchContent)
FetchContent_Declare(
    CMSISDSP
    GIT_REPOSITORY https://github.com/ARM-software/CMSIS-DSP.git
    # maybe use hash instead
    GIT_TAG        v1.16.2
    GIT_SHALLOW    1
)

# needed by CMSISDSP
set(CMSISCORE ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS)
FetchContent_MakeAvailable(CMSISDSP)

target_compile_options(CMSISDSP PUBLIC 
    -Wsign-compare
    -Wdouble-promotion
    -Ofast -ffast-math
    -DNDEBUG
    -fshort-enums
    #-fshort-wchar
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    CMSISDSP
)

add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME}
    POST_BUILD

    COMMAND ${CMAKE_OBJCOPY}
    ARGS -O binary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin
)
