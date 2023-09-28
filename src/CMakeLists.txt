cmake_minimum_required(VERSION 3.22.1)

project(NES VERSION 0.1)

configure_file(NESConfig.h.in NESConfig.h)

add_executable(${PROJECT_NAME} nes.c devices/display.c debug/pattern_table.c debug/name_table.c processing/palette.c processing/apu.c devices/controller.c debug/debug.c debug/debug_panel.c memory/mapper.c processing/cpu.c processing/ppu.c memory/mem.c memory/ram.c memory/rom.c memory/vram.c)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} SDL2 SDL2main SDL2_ttf)

install(TARGETS NES DESTINATION bin)