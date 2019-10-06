cmake_minimum_required(VERSION 3.0) # setting this is required
project(spinner)            # this sets the project name

set(assets_path "${CMAKE_CURRENT_BINARY_DIR}/../../../assets/")

file(GLOB_RECURSE sources
    spinner.c
    ../../common/framebuffer.cc
    ../../common/spinner.c
    ../../../phonelibs/nanovg/nanovg.c
)


add_executable(spinner ${sources} link_data.o opensans_semibold.o img_spinner_track.o img_spinner_comma.o)

target_compile_options(spinner PUBLIC -std=c++11 -Wall -fPIC)
target_include_directories(spinner PUBLIC
    ../../
    ../../../phonelibs/nanovg/
    )

target_link_libraries(spinner   EGL GLESv2 X11 m)

add_custom_command(OUTPUT link_data.o
      COMMAND ld -r -b binary -o opensans_semibold.o ${assets_path}/fonts/opensans_semibold.ttf
      COMMAND ld -r -b binary -o img_spinner_track.o ${assets_path}/img_spinner_track.png
      COMMAND ld -r -b binary -o img_spinner_comma.o ${assets_path}/img_spinner_comma.png

      COMMAND objcopy --rename-section .data=.rodata,alloc,load,readonly,data,contents opensans_semibold.o link_data.o
)
