# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

if(WITH_PPL)
  include(${VTK_USE_FILE})

  set(SRC
    main.cpp
  )

  set(HEADERS

  )

  add_executable(ppl_test ${SRC})

  target_link_libraries(ppl_test
    ${LIBS}
    ${LIBS_PPL}
    ${LIBS_VTK}
    ${LIBS_GMP}
  )
endif()


# Tests

 #  if(BUILD_TESTS)
	# add_test(NAME ExampleArithmetics COMMAND ex_arithmetic 0)
 #  endif(BUILD_TESTS)
