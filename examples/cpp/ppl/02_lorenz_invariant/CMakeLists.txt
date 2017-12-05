# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

  set(SRC
	main.cpp
  )

  set(HEADERS

  )

  add_executable(ppl_3D_invariant_lorenz ${SRC})

  target_link_libraries(ppl_3D_invariant_lorenz
    ${LIBS}
    invariant-core
    invariant-graphics-3D
  )


# Tests

 #  if(BUILD_TESTS)
	# add_test(NAME ExampleArithmetics COMMAND ex_arithmetic 0)
 #  endif(BUILD_TESTS)
