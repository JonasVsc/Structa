cmake_minimum_required (VERSION 3.31)
project ("Structa")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(Vulkan)
add_subdirectory(3rdparty/cimgui)

include_directories(.)

# --- Module Game --- #
add_library(structa_game SHARED
    "game/game.c"
)

target_link_libraries(structa_game PUBLIC Vulkan::Vulkan)

# --- Module GUI --- #
add_library(structa_gui_module SHARED
    "modules/gui/gui_module.h"
    "modules/gui/gui_module.c"
)

target_link_libraries(structa_gui_module PUBLIC cimgui)

# --- Structa --- #
include_directories(structa/src)
include_directories(shared)

add_executable(structa WIN32
	"structa/src/structa_internal.h" 
	"structa/src/structa_loader.h"
	"structa/src/structa_loader.c"
	"structa/src/structa_modules.h"
	"structa/src/structa_modules.c"
	"structa/src/structa_window.h" 
	"structa/src/structa_window.c" 
	"structa/src/structa_renderer.h" 
	"structa/src/structa_renderer.c" 
	"structa/src/structa_context.h"
	"structa/src/structa_context.c"
	"structa/src/structa_entry.c"
	"shared/structa_vulkan.c"
	"shared/structa_utils.c"
)

target_link_libraries(structa PUBLIC Vulkan::Vulkan)
target_include_directories(structa PUBLIC structa/src structa/include)

# --- GLSL --- #

find_program(
    GLSLC_EXECUTABLE glslc
    REQUIRED
    DOC "Caminho para o compilador glslc"
)