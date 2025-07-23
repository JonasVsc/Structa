cmake_minimum_required (VERSION 3.31)
project ("Structa")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(Vulkan)

set(STRUCTA_INTERNAL
    "structa_internal.c"
)

set(STRUCTA_CORE
    "structa_window.c"
    "structa_renderer.c"
)

add_library(structa_lib SHARED
    ${STRUCTA_INTERNAL}
    ${STRUCTA_CORE}
)

target_link_libraries(structa_lib
	PUBLIC Vulkan::Vulkan
)

add_executable(structa_app WIN32
    "main.c"
)

target_link_libraries(structa_app PRIVATE structa_lib)

# --- GLSL --- #

find_program(
    GLSLC_EXECUTABLE glslc
    REQUIRED
    DOC "Caminho para o compilador glslc"
)