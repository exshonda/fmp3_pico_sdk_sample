set(FMP3_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(TOPPERS_OMIT_CHIP_SERIAL true)

if (PICO_PLATFORM STREQUAL "rp2040")
    set(FMP3_TARGET rp2040_pico_sdk)
elseif (PICO_PLATFORM STREQUAL "rp2350-arm-s")
    set(FMP3_TARGET rp2350-arm-s_pico_sdk)
elseif (PICO_PLATFORM STREQUAL "rp2350-riscv")
    message(FATAL_ERROR "not suported ${PICO_PLATFORM}")
else()
    message(FATAL_ERROR "not suported ${PICO_PLATFORM}")
endif()

list(APPEND FMP3_COMPILE_DEFS
    OMIT_ISTACK
    TOPPERS_OMIT_VECTOR_TABLE
)

function(fmp3_set_pico_sdk_options TARGET)
  target_compile_definitions(${TARGET}
    PUBLIC PICO_RUNTIME_NO_INIT_INSTALL_RAM_VECTOR_TABLE
    PUBLIC PICO_RUNTIME_SKIP_INIT_INSTALL_RAM_VECTOR_TABLE
    PUBLIC PICO_RUNTIME_SKIP_INIT_PER_CORE_IRQ_PRIORITIES
  )

  target_link_options(${TARGET} 
    PRIVATE "LINKER:--wrap=exception_get_vtable_handler"
    PRIVATE "LINKER:--wrap=exception_set_exclusive_handler"
    PRIVATE "LINKER:--wrap=exception_restore_handler"
    PRIVATE "LINKER:--wrap=irq_get_vtable_handler"
    PRIVATE "LINKER:--wrap=irq_set_exclusive_handler"
    PRIVATE "LINKER:--wrap=irq_add_shared_handler"
    PRIVATE "LINKER:--wrap=irq_add_shared_handler"
    PRIVATE "LINKER:--wrap=irq_remove_handler"
    PRIVATE "LINKER:--wrap=irq_add_tail_to_free_list"
  )
endfunction()
