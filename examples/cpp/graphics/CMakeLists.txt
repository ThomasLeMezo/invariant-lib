# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

  set(SRC
	main.cpp
  )

  set(HEADERS

  )

  add_executable(graphics_example ${SRC})

  target_link_libraries(graphics_example
	 ${LIBS}
	 invariant-core
         invariant-tools
         invariant-graphics
  )


# Tests

 #  if(BUILD_TESTS)
	# add_test(NAME ExampleArithmetics COMMAND ex_arithmetic 0)
 #  endif(BUILD_TESTS)
