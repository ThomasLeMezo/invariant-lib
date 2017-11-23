# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

  set(SRC
	main.cpp
  )

  set(HEADERS

  )

  add_executable(2D_integration_full_cones ${SRC})

  target_link_libraries(2D_integration_full_cones
	${LIBS}
	invariant-core
	
	invariant-graphics
  )


# Tests

 #  if(BUILD_TESTS)
	# add_test(NAME ExampleArithmetics COMMAND ex_arithmetic 0)
 #  endif(BUILD_TESTS)
