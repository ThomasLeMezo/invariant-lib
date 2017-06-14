# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

include_directories(${CMAKE_SOURCE_DIR}/src/invariant/)

  set(SRC
    vibes/vibes.cpp
    VibesFigure.cpp
    VibesFigure_Graph.cpp
    graphiz_graph.cpp
  )

  set(HEADERS
    vibes/vibes.h
    graphiz_graph.h
    VibesFigure.h
    VibesFigure_Graph.h
  )

  file(COPY ${HEADERS} DESTINATION ${HEADERS_DIR})

  add_library(invariant-graphics ${SRC})
  target_link_libraries(invariant-graphics ${LIBS})

  install(FILES ${HEADERS} DESTINATION "include/invariant-lib")
  install(TARGETS invariant-graphics DESTINATION lib)
