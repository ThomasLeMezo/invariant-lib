# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================


# Sources
include_directories(definition)
include_directories(graph)
include_directories(maze)
include_directories(../tools/serialization)


  set(SRC
    graph/graph.cpp
    graph/pave.cpp
    graph/face.cpp
    graph/pave_node.cpp

    maze/maze.cpp
    maze/room.cpp
    maze/door.cpp

    maze/propagator/mazepropagator.cpp
    maze/propagator/roompropagator.cpp
    maze/propagator/doorpropagator.cpp

    maze/contractor/mazecontractor.cpp
    maze/contractor/roomcontractor.cpp
    maze/contractor/doorcontractor.cpp

    definition/domain.cpp
    definition/dynamics.cpp
    definition/dynamics_function.cpp
  )

  set(HEADERS
    graph/graph.h
    graph/pave.h
    graph/face.h
    graph/pave_node.h

    maze/maze.h
    maze/room.h
    maze/door.h

    maze/propagator/mazepropagator.h
    maze/propagator/roompropagator.h
    maze/propagator/doorpropagator.h

    maze/contractor/mazecontractor.h
    maze/contractor/roomcontractor.h
    maze/contractor/doorcontractor.h

    definition/domain.h
    definition/dynamics.h
    definition/dynamics_function.cpp
  )

  file(COPY ${HEADERS} DESTINATION ${HEADERS_DIR})

  add_library(invariant-core ${SRC})
  target_link_libraries(invariant-core ${LIBS} invariant-tools)

  install(FILES ${HEADERS} DESTINATION "include/invariant-lib")
  install(TARGETS invariant-core DESTINATION lib)
