# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

  set(SRC
	main.cpp
  )

  set(HEADERS

  )

  add_executable(2D_invariant_van_der_pol_positive ${SRC})

  target_link_libraries(2D_invariant_van_der_pol_positive
	 ${LIBS}
	 invariant-core
         invariant-tools
         invariant-graphics
  )


# Tests

 #  if(BUILD_TESTS)
	# add_test(NAME ExampleArithmetics COMMAND ex_arithmetic 0)
 #  endif(BUILD_TESTS)
