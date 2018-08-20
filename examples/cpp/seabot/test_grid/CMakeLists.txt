# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

set(SRC
    main.cpp
    )

set(HEADERS

    )

add_executable(SEABOT_test_grid ${SRC})

target_link_libraries(SEABOT_test_grid
    ${LIBS}
    invariant-core
    invariant-graphics
)


# Tests

#  if(BUILD_TESTS)
# add_test(NAME ExampleArithmetics COMMAND ex_arithmetic 0)
#  endif(BUILD_TESTS)
