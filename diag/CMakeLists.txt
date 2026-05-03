project(diag C CXX ASM)

# Kompilujemy lcdspi.c bezpośrednio w diag zamiast linkować bibliotekę
# żeby uniknąć duplikacji symboli pico_stdlib
add_executable(diag
    diag.c
    ../common/lcdspi/lcdspi.c
)

pico_set_program_name(diag "bl-diagnostic")
pico_set_program_description(diag "Diagnostics for uf2loader")
pico_set_program_version(diag "${VERSION_STRING}")

pico_enable_stdio_uart(diag 0)
pico_enable_stdio_usb(diag 0)
pico_enable_stdio_rtt(diag 0)

pico_minimize_runtime(diag)

if (PICO_BOARD MATCHES "^pico(_w)?$")
  set_target_properties(diag PROPERTIES OUTPUT_NAME "diag_pico")
else()
  set_target_properties(diag PROPERTIES OUTPUT_NAME "diag_pico2")
endif()

pico_add_extra_outputs(diag)

target_compile_definitions(diag PRIVATE
    #ENABLE_DEBUG=1
)

target_link_libraries(diag
    pico_stdlib
    hardware_gpio
    hardware_spi
    petit_fatfs
    # lcdspi USUNIĘTE - kompilujemy .c bezpośrednio
)

target_include_directories(diag PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../common/lcdspi
)

target_link_options(diag PRIVATE -Wl,--print-memory-usage)
