# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

if(WITH_3D)
  set(SRC
	main.cpp
  )

  set(HEADERS

  )

  add_executable(3D_invariant_corkscrew ${SRC})

  target_link_libraries(3D_invariant_corkscrew
    ${LIBS}
    invariant-core
    invariant-graphics-3D
				invariant-language
  )
endif()

# Tests

 #  if(BUILD_TESTS)
	# add_test(NAME ExampleArithmetics COMMAND ex_arithmetic 0)
 #  endif(BUILD_TESTS)
