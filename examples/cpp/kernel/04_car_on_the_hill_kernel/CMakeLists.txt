# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

  set(SRC
	main.cpp
  )

  set(HEADERS

  )

  add_executable(2D_kernel_car_on_the_hill ${SRC})

  target_link_libraries(2D_kernel_car_on_the_hill
	${LIBS}
	invariant-core
	
	invariant-graphics
  )


# Tests

 #  if(BUILD_TESTS)
	# add_test(NAME ExampleArithmetics COMMAND ex_arithmetic 0)
 #  endif(BUILD_TESTS)
