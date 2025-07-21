cmake_minimum_required (VERSION 3.31)
project ("Structa")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(Vulkan)

add_library(structa_lib SHARED
    "structa_window.c"
)

target_link_libraries(structa_lib
	PUBLIC Vulkan::Vulkan
)

add_executable(structa_app WIN32
    "structa_app.c"
)

target_link_libraries(structa_app PRIVATE structa_lib)

# --- GLSL --- #

find_program(
    GLSLC_EXECUTABLE glslc
    REQUIRED
    DOC "Caminho para o compilador glslc"
)