# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================


# Sources
include_directories(definition)
include_directories(definition/netcdf)
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
    maze/sepmaze.cpp

    definition/domain.cpp
    definition/dynamics.cpp
    definition/dynamics_function.cpp
  )

  if(WITH_NETCDF)
    set(SRC ${SRC}
      definition/netcdf/nodecurrent.cpp
      definition/netcdf/previmer.cpp
      definition/netcdf/previmer3d.cpp
      definition/netcdf/nodecurrent3d.cpp
    )
  endif()

  set(HEADERS
    graph/graph.h
    graph/pave.h
    graph/face.h
    graph/pave_node.h

    maze/maze.h
    maze/room.h
    maze/door.h
    maze/sepmaze.h

    definition/domain.h
    definition/dynamics.h
    definition/dynamics_function.h
  )

  if(WITH_NETCDF)
    set(HEADERS ${HEADERS}
      definition/netcdf/nodecurrent.h
      definition/netcdf/previmer.h
      definition/netcdf/previmer3d.h
      definition/netcdf/nodecurrent3d.h
    )
  endif()

  file(COPY ${HEADERS} DESTINATION ${HEADERS_DIR})

  add_library(invariant-core ${SRC} ${HEADERS})
  target_link_libraries(invariant-core
      ${LIBS}
      ${LIBS_NETCDF}
      invariant-tools
  )

  install(FILES ${HEADERS} DESTINATION "include/invariant-lib")
  install(TARGETS invariant-core DESTINATION lib)
