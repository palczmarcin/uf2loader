cmake_minimum_required(VERSION 3.13)

# Wersja
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# Pico SDK
include(pico_sdk_import.cmake)

project(uf2loader C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# --- Pimoroni Pico SDK ---
# Dodaj pimoroni-pico jako submoduł
set(PIMORONI_PICO_PATH ${CMAKE_CURRENT_LIST_DIR}/pimoroni-pico)
set(PICO_BOARD_HEADER_DIRS ${PIMORONI_PICO_PATH})

include(${PIMORONI_PICO_PATH}/pimoroni_pico_import.cmake)

# Tylko potrzebne komponenty Pimoroni
include(${PIMORONI_PICO_PATH}/drivers/st7789/st7789)
include(${PIMORONI_PICO_PATH}/libraries/pico_graphics/pico_graphics)

# --- Podkatalogi uf2loader ---
add_subdirectory(common/bootloader)
add_subdirectory(common/lcdspi)
add_subdirectory(common/petit_fatfs)
add_subdirectory(stage3)
add_subdirectory(ui)
add_subdirectory(diag)
