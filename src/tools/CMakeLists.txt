# ==================================================================
#  tubex-lib - cmake configuration file
# ==================================================================

# Sources

  set(SRC
    serialization/serialization.cpp
    netcdf/previmer.cpp
  )

  set(HEADERS
    serialization/serialization.h
    netcdf/previmer.h
  )

  file(COPY ${HEADERS} DESTINATION ${HEADERS_DIR})

  add_library(invariant-tools ${SRC})
  target_link_libraries(invariant-tools ${LIBS})

  install(FILES ${HEADERS} DESTINATION "include/invariant-lib")
  install(TARGETS invariant-tools DESTINATION lib)
