# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================


# Sources

  set(SRC
    # ToDo
  )

  set(HEADERS
    # ToDo
  )

  include_directories(vibes)
  file(COPY ${HEADERS} DESTINATION ${HEADERS_DIR})

  add_library(invariant-graphics ${SRC})
  target_link_libraries(invariant-graphics ${LIBS})

  install(FILES ${HEADERS} DESTINATION "include/invariant-lib")
  install(TARGETS invariant-graphics DESTINATION lib)