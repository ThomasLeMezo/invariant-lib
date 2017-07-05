# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

  set(SRC
	main.cpp
  )

  set(HEADERS

  )

  add_executable(2D_integration_van_der_pol ${SRC})

  target_link_libraries(2D_integration_van_der_pol
	 ${LIBS}
	 invariant-core
         invariant-tools
         invariant-graphics
  )


# Tests

 #  if(BUILD_TESTS)
	# add_test(NAME ExampleArithmetics COMMAND ex_arithmetic 0)
 #  endif(BUILD_TESTS)
