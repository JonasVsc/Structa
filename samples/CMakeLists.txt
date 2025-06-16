
# Side scroller sample
add_executable(side-scroller
	"side-scroller/main.c"
)

target_link_libraries(side-scroller 
	PRIVATE Structa
)

# Top down sample
add_executable(top-down
	"top-down/main.c"
)

target_link_libraries(top-down 
	PRIVATE Structa
)