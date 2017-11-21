# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

if(WITH_NETCDF AND WITH_3D)

  set(SRC
    main.cpp
  )

  set(HEADERS

  )

  add_executable(3D_netcdf_test ${SRC})

  target_link_libraries(3D_netcdf_test
    ${LIBS}
    ${LIBS_NETCDF}
    invariant-core
    invariant-tools
    invariant-graphics-3D
  )
endif()


# Tests

 #  if(BUILD_TESTS)
	# add_test(NAME ExampleArithmetics COMMAND ex_arithmetic 0)
 #  endif(BUILD_TESTS)
