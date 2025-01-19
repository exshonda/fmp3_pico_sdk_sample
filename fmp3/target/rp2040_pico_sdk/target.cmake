
set(ARCHDIR ${PROJECT_SOURCE_DIR}/arch/arm_m_gcc)
set(TARGETDIR ${PROJECT_SOURCE_DIR}/target/rp2040_pico_sdk)

list(APPEND FMP3_CFG_FILES
    ${TARGETDIR}/target_kernel.cfg
)

list(APPEND FMP3_CLASS_TRB_FILES
    ${TARGETDIR}/target_class.trb
)

list(APPEND FMP3_KERNEL_CFG_TRB_FILES
    ${TARGETDIR}/target_kernel.trb
)

list(APPEND FMP3_CHECK_TRB_FILES
    ${TARGETDIR}/target_check.trb
)

list(APPEND FMP3_INCLUDE_DIRS
    ${CMAKE_BINARY_DIR}/generated/pico_base
    ${PICO_SDK_PATH}/src/common/pico_base_headers/include
    ${PICO_SDK_PATH}/src/common/hardware_claim/include
    ${PICO_SDK_PATH}/src/common/pico_sync/include
    ${PICO_SDK_PATH}/src/common/pico_time/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_platform_compiler/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_platform_sections/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_platform_panic/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_runtime_init/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_runtime/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_multicore/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_base/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_exception/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_irq/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_sync/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_sync_spin_lock/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_timer/include
    ${PICO_SDK_PATH}/src/rp2040/pico_platform/include
    ${PICO_SDK_PATH}/src/rp2040/hardware_regs/include
    ${PICO_SDK_PATH}/src/rp2040/hardware_structs/include
    ${TARGETDIR}
)

list(APPEND FMP3_COMPILE_DEFS
    PICO_RP2040
    USE_TIM_AS_HRT
)

list(APPEND FMP3_TARGET_C_FILES
    ${TARGETDIR}/target_kernel_impl.c
    ${TARGETDIR}/target_timer.c
    ${TARGETDIR}/target_serial.c
)

include(${ARCHDIR}/rp2040/arch.cmake)
