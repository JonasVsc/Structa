set(STRUCTA_INTERNAL
    "src/structa_internal.c"
)

set(STRUCTA_CORE
    "src/structa_window.c"
    "src/structa_renderer.c"
)

set(STRUCTA_SHARED
    "src/shared/structa_vk_init.c"
)

# --- SYSTEMS ---
set(STRUCTA_GUI
    "src/systems/structa_gui/structa_internal_gui.c"
    "src/systems/structa_gui/structa_gui.c"
)

add_library(structa_lib SHARED
    ${STRUCTA_INTERNAL}
    ${STRUCTA_SHARED}
    ${STRUCTA_CORE}
    
    # SYSTEMS
    ${STRUCTA_GUI}
    "src/structa_utils.c"
    
    "src/structa_experimental.c"
)

target_include_directories(structa_lib 
    PUBLIC src include
)

target_link_libraries(structa_lib
	PUBLIC Vulkan::Vulkan
)