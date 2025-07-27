

# ---- STRUCTA MODULES ----
set(STRUCTA_GUI
    "src/modules/structa_gui/structa_gui.c"
)



# ---- STRUCTA LIB ----
set(STRUCTA_CORE
    "src/core/structa_context.c"
    "src/core/window/structa_window.c"
    "src/core/renderer/structa_renderer.c"
)

set(STRUCTA_COMMON
    "src/common/utils.c"
    "src/common/vk_initializers.c"
)

add_library(structa_lib SHARED
    ${STRUCTA_COMMON}
    ${STRUCTA_CORE}
    ${STRUCTA_GUI}
    "src/structa_experimental.c"
)

target_include_directories(structa_lib 
    PUBLIC src include
)

target_link_libraries(structa_lib
	PUBLIC Vulkan::Vulkan
)