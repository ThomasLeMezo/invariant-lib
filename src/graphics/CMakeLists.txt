# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

include_directories(../invariant/graph)
include_directories(../invariant/maze)
include_directories(../invariant/definition)
include_directories(vibes)
# include(${VTK_USE_FILE})

  set(SRC
    vibes/vibes.cpp
    VibesFigure.cpp
    vibes_graph.cpp
    graphiz_graph.cpp
#    vtk_graph.cpp
  )

  set(HEADERS
    vibes/vibes.h
    graphiz_graph.h
    VibesFigure.h
    vibes_graph.h
#    vtk_graph.h
  )

  file(COPY ${HEADERS} DESTINATION ${HEADERS_DIR})

  add_library(invariant-graphics ${SRC})
  target_link_libraries(invariant-graphics ${LIBS})

  install(FILES ${HEADERS} DESTINATION "include/invariant-lib")
  install(TARGETS invariant-graphics DESTINATION lib)
