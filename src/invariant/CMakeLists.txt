# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================


# Sources
include_directories(graph)
include_directories(maze)
include_directories(../tools)

  set(SRC
    graph/graph.cpp
    graph/pave.cpp
    graph/face.cpp
    graph/pave_node.cpp

    maze/maze.cpp
    maze/room.cpp
    maze/door.cpp
  )

  set(HEADERS
    graph/graph.h
    graph/pave.h
    graph/face.h
    graph/pave_node.h

    maze/maze.h
    maze/room.h
    maze/door.h
  )

  file(COPY ${HEADERS} DESTINATION ${HEADERS_DIR})

  add_library(invariant-core ${SRC})
  target_link_libraries(invariant-core ${LIBS} invariant-tools)

  install(FILES ${HEADERS} DESTINATION "include/invariant-lib")
  install(TARGETS invariant-core DESTINATION lib)
