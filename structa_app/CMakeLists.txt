add_executable(structa_app WIN32
    "src/app.c"
)

target_link_libraries(structa_app PRIVATE structa_lib)


find_program(
    GLSLC_EXECUTABLE glslc
    REQUIRED
    DOC "Caminho para o compilador glslc"
)

add_custom_command(
    TARGET structa_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/structa_lib/shaders"
            "$<TARGET_FILE_DIR:structa_app>/shaders"
    COMMENT "Copiando diretório de shaders para o diretório do executável"
)

add_custom_command(TARGET structa_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:structa_lib>"
        "$<TARGET_FILE_DIR:structa_app>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_PDB_FILE:structa_lib>"
        "$<TARGET_FILE_DIR:structa_app>"
)