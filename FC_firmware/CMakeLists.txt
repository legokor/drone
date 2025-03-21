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

# CMSIS DSP
set(CMSISCORE ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS) # /Device/ST/STM32F4xx)
set(CMSISDSP ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/DSP)
add_subdirectory(${CMSISDSP}/Source build/cmsis_dsp)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Drivers/gps/impl/gps.c
    Drivers/mag/impl/mag.c
    Drivers/acc/impl/acc.c
    Drivers/imu/impl/imu.c
    Drivers/bar/impl/bar.c
    Drivers/lora/impl/lora.c
    Drivers/esc/impl/esc.c
    Drivers/rc/impl/rc.c
    Drivers/sd/impl/sd.c
    Drivers/uart/impl/uart.c
    Drivers/usb/impl/usb.c

    Src/ctrl/impl/ctrl.c
    Src/dsp/impl/dsp.c
    Src/act/impl/act.c
    Src/llc/impl/llc.c
    Src/tel/impl/tel.c
    Src/guide/impl/guide.c
    Src/sys/impl/sys.c
    Src/log/impl/log.c
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

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    CMSISDSP
)
