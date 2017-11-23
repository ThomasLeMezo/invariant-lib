# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================


# Sources
include_directories(definition)
include_directories(definition/netcdf)
include_directories(definition/dataSet)
include_directories(serialization)
include_directories(smartSubPaving)
include_directories(maze)

  set(SRC
    smartSubPaving/smartSubPaving.cpp
    smartSubPaving/pave.cpp
    smartSubPaving/face.cpp
    smartSubPaving/pave_node.cpp

    maze/maze.cpp
    maze/room.cpp
    maze/door.cpp
    maze/sepmaze.cpp

    definition/domain.cpp
    definition/dynamics.cpp
    definition/dynamics_function.cpp

    serialization/ibex_serialization.cpp
  )

  if(WITH_NETCDF)
    set(SRC ${SRC}
      definition/netcdf/nodecurrent.cpp
      definition/netcdf/previmer.cpp
      definition/netcdf/previmer3d.cpp

      definition/dataSet/datasetfunction.cpp
    )
  endif()

  set(HEADERS
    smartSubPaving/smartSubPaving.h
    smartSubPaving/pave.h
    smartSubPaving/face.h
    smartSubPaving/pave_node.h

    maze/maze.h
    maze/room.h
    maze/door.h
    maze/sepmaze.h

    definition/domain.h
    definition/dynamics.h
    definition/dynamics_function.h

    serialization/ibex_serialization.h
  )

  if(WITH_NETCDF)
    set(HEADERS ${HEADERS}
      definition/netcdf/nodecurrent.h
      definition/netcdf/previmer.h
      definition/netcdf/previmer3d.h

      definition/dataSet/datasetnode.h
      definition/dataSet/datasetnode.tpp

      definition/dataSet/datasetfunction.h
      definition/dataSet/datasetvirtualnode.h
    )
  endif()

  file(COPY ${HEADERS} DESTINATION ${HEADERS_DIR})

  add_library(invariant-core
      ${SRC}
      ${HEADERS}
  )
  target_link_libraries(invariant-core
      ${LIBS}
      ${LIBS_NETCDF}
  )

  install(FILES ${HEADERS} DESTINATION "include/invariant-lib")
  install(TARGETS invariant-core DESTINATION lib)
