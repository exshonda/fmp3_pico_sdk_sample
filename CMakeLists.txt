# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(fmp3_pico_sdk C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# FMP3のpico-SDK用のCMakeファイルをインクルード
include(fmp3/fmp3_pico_sdk.cmake)

# Add executable. Default name is the project name, version 0.1

add_executable(fmp3_pico_sdk
    fmp3_pico_sdk.c
    fmp3/library/log_output.c
    fmp3/library/vasyslog.c
    fmp3/syssvc/syslog.c
    fmp3/syssvc/banner.c
)

pico_set_program_name(fmp3_pico_sdk "fmp3_pico_sdk")
pico_set_program_version(fmp3_pico_sdk "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(fmp3_pico_sdk 0)
pico_enable_stdio_usb(fmp3_pico_sdk 1)

# FMP3の設定
set(FMP3_APP_CFG_FILE ${PROJECT_SOURCE_DIR}/fmp3_pico_sdk.cfg)

list(APPEND FMP3_INCLUDE_DIRS
    ${FMP3_ROOT_DIR}/sample
)

add_subdirectory(fmp3)

include(fmp3/library/library.cmake)
include(fmp3/syssvc/syssvc.cmake)

# Add the standard library to the build
target_link_libraries(fmp3_pico_sdk
    pico_stdlib
    pico_multicore
    fmp3
)

# Add the standard include files to the build
target_include_directories(fmp3_pico_sdk PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${PROJECT_SOURCE_DIR}
)

fmp3_set_pico_sdk_options(fmp3_pico_sdk)

pico_add_extra_outputs(fmp3_pico_sdk)

#fmp3_cfg_check(fmp3_pico_sdk)
