# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

if(WITH_GRAPHIZ)
  set(SRC
	main.cpp
  )

  set(HEADERS

  )

  add_executable(2D_graphics_example ${SRC})

  target_link_libraries(2D_graphics_example
         ${LIBS}
         ${LIBS_GRAPHIZ}
	 invariant-core
         invariant-graphics-graphiz
         invariant-graphics
  )
endif()


# Tests

 #  if(BUILD_TESTS)
	# add_test(NAME ExampleArithmetics COMMAND ex_arithmetic 0)
 #  endif(BUILD_TESTS)
