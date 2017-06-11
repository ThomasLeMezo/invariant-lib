# ==================================================================
#  tubex-lib - cmake configuration file
# ==================================================================


# Sources

  set(SRC
    serialization.cpp
  )

  set(HEADERS
    serialization.h
  )

  file(COPY ${HEADERS} DESTINATION ${HEADERS_DIR})

  add_library(invariant-tools ${SRC})
  target_link_libraries(invariant-tools ${LIBS})

  install(FILES ${HEADERS} DESTINATION "include/invariant-lib")
  install(TARGETS invariant-tools DESTINATION lib)
