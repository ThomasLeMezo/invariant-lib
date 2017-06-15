# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================


# Sources

  set(SRC
    graph.cpp
    pave.cpp
    face.cpp
    pave_node.cpp
  )

  set(HEADERS
    graph.h
    pave.h
    face.h
    pave_node.h
  )

  file(COPY ${HEADERS} DESTINATION ${HEADERS_DIR})

  add_library(invariant-core ${SRC})
  target_link_libraries(invariant-core ${LIBS} invariant-tools)

  install(FILES ${HEADERS} DESTINATION "include/invariant-lib")
  install(TARGETS invariant-core DESTINATION lib)
