# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================


set(TESTS_CTC_SOURCES
  tests_graph.cpp
)

add_executable(invariantlib_tests_graph ${TESTS_CTC_SOURCES})
target_link_libraries(invariantlib_tests_graph invariant-core)
add_test(NAME TestGraph COMMAND invariantlib_tests_graph)