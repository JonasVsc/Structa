cmake_minimum_required (VERSION 3.31)
project ("Structa")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(Vulkan)

include_directories(src)

# --- Structa --- #

set(STRUCTA_SOURCE 
    "src/structa_enum.h"
    "src/structa_context.h"
    "src/structa_context.c"
    "src/structa_loader.h"
    "src/structa_table.h"
    "src/structa_modules.h"
    "src/structa_modules.c")

add_library(structa_lib STATIC
    ${STRUCTA_SOURCE})

target_link_libraries(structa_lib PUBLIC
    Vulkan::Vulkan)

target_include_directories(structa_lib PUBLIC
    include)

# --- Gui module --- #

set(GUI_SOURCE
    "src/gui/gui_module.h"
    "src/gui/gui_module.c")

add_library(structa_gui SHARED
    ${GUI_SOURCE})

target_link_libraries(structa_gui PRIVATE
    structa_lib)

target_include_directories(structa_gui PRIVATE 
    include)

# --- Main --- #

add_executable(structa_app WIN32
    "main.c")

target_link_libraries(structa_app PRIVATE 
    structa_lib)

# --- GLSL --- #

find_program(
    GLSLC_EXECUTABLE glslc
    REQUIRED
    DOC "Caminho para o compilador glslc"
)