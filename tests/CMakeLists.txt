add_executable(run_tests "test_main.c")

target_link_libraries(run_tests PRIVATE Structa unity)

add_test(NAME unit_tests COMMAND run_tests)

