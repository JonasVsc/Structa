cmake_minimum_required (VERSION 3.31)
project ("Structa")

include(FetchContent)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(Vulkan)

FetchContent_Declare(
    SDL
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.32.8 
)

set(SDL_TESTS OFF CACHE BOOL "Disable SDL tests")
set(SDL_INSTALL_TESTS OFF CACHE BOOL "Disable installing SDL tests")

FetchContent_MakeAvailable(SDL)

FetchContent_Declare(
    CGLM
    GIT_REPOSITORY https://github.com/recp/cglm.git
    GIT_TAG        v0.9.6
)

FetchContent_MakeAvailable(CGLM)

set(RENDERER_SRC
	"src/renderer.c"
)

set(UTILS_SRC
	"src/utils/helper.c"
)

set(GAMEBASE_SRC
	"src/gamebase.c"
)

add_library (${PROJECT_NAME} SHARED
	${GAMEBASE_SRC}
	${RENDERER_SRC}
	${UTILS_SRC}
)

target_include_directories(${PROJECT_NAME}
	PUBLIC Vulkan::Headers include src
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC Vulkan::Vulkan SDL2 SDL2main cglm
)

# --- Shaders --- #

find_program(
    GLSLC_EXECUTABLE glslc
    REQUIRED
    DOC "Caminho para o compilador glslc"
)

set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/samples")
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

file(GLOB_RECURSE SHADER_SOURCES "src/shaders/*.vert" "src/shaders/*.frag")

foreach(SHADER_SOURCE_FILE ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER_SOURCE_FILE} NAME)
    set(SHADER_OUTPUT_FILE "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv")

    add_custom_command(
        OUTPUT ${SHADER_OUTPUT_FILE}
        COMMAND ${GLSLC_EXECUTABLE} ${SHADER_SOURCE_FILE} -o ${SHADER_OUTPUT_FILE}
        DEPENDS ${SHADER_SOURCE_FILE}
        COMMENT "Compilando shader: ${SHADER_NAME}"
    )

    list(APPEND SHADER_OUTPUTS ${SHADER_OUTPUT_FILE})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${SHADER_OUTPUTS})

# --- Samples --- #

add_subdirectory(samples)

add_custom_command(
    TARGET Structa POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:Structa>"
        "$<TARGET_FILE_DIR:Structa>/samples"
)

# --- Test --- #

enable_testing()

FetchContent_Declare(
    Unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.6.1 
)

FetchContent_MakeAvailable(Unity)

add_subdirectory(tests)

add_custom_command(
    TARGET Structa POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:Structa>"
        "$<TARGET_FILE_DIR:Structa>/tests"
)
