# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================


set(TESTS_CTC_SOURCES
  tests_graph.cpp
)

add_executable(TEST_invariantlib_graph ${TESTS_CTC_SOURCES})

target_link_libraries(TEST_invariantlib_graph
    invariant-core
)

add_test(NAME TestGraph COMMAND TEST_invariantlib_graph)
