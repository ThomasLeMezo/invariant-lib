# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

if(WITH_NETCDF)
  set(SRC
    main.cpp
  )

  set(HEADERS

  )

  add_executable(2D_netcdf_test ${SRC})

  target_link_libraries(2D_netcdf_test
    ${LIBS}
    ${LIBS_NETCDF}
    invariant-core
    invariant-tools
    invariant-graphics
  )
endif()


# Tests

 #  if(BUILD_TESTS)
	# add_test(NAME ExampleArithmetics COMMAND ex_arithmetic 0)
 #  endif(BUILD_TESTS)
