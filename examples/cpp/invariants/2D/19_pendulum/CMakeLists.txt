# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

  set(SRC
	 main.cpp
  )

  set(HEADERS

  )

  add_executable(2D_invariant_pendulum ${SRC})

  target_link_libraries(2D_invariant_pendulum
    ${LIBS}
    invariant-core
    
    invariant-graphics
  )


# Tests

 #  if(BUILD_TESTS)
	# add_test(NAME ExampleArithmetics COMMAND ex_arithmetic 0)
 #  endif(BUILD_TESTS)
