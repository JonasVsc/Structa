
# Side scroller sample
add_executable(side-scroller
	"side-scroller/main.c"
)

target_link_libraries(side-scroller 
	PRIVATE Structa
)

add_custom_command(
    TARGET side-scroller POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:Structa>"
        "$<TARGET_FILE_DIR:side-scroller>"
)

# Top down sample
add_executable(top-down
	"top-down/main.c"
)

target_link_libraries(top-down 
	PRIVATE Structa
)

add_custom_command(
    TARGET side-scroller POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:Structa>"
        "$<TARGET_FILE_DIR:top-down>"
)