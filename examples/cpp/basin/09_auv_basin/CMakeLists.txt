# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

  set(SRC
	main.cpp
  )

  set(HEADERS

  )

  add_executable(2D_basin_auv ${SRC})

  target_link_libraries(2D_basin_auv
    ${LIBS}
    invariant-core
    invariant-graphics
    invariant-graphics-3D
  )


# Tests

 #  if(BUILD_TESTS)
	# add_test(NAME ExampleArithmetics COMMAND ex_arithmetic 0)
 #  endif(BUILD_TESTS)
