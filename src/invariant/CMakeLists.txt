# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================


# Sources
include_directories(serialization)
include_directories(definition)
include_directories(definition/netcdf)
include_directories(definition/dataSet)
include_directories(smartSubPaving)
include_directories(maze)

  set(SRC
    definition/dynamics.cpp
    definition/dynamics_function.cpp

    serialization/ibex_serialization.cpp

    maze/room.cpp
    maze/door.cpp
    smartSubPaving/face.cpp

    smartSubPaving/pave.cpp
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
    serialization/ibex_serialization.h

    maze/maze.h
    maze/room.h
    maze/door.h
    maze/sepmaze.h

    smartSubPaving/smartSubPaving.h
    smartSubPaving/pave.h
    smartSubPaving/face.h
    smartSubPaving/pave_node.h

    definition/domain.h
    definition/dynamics.h
    definition/dynamics_function.h

    smartSubPaving/smartSubPaving.tpp
    smartSubPaving/pave.tpp
    smartSubPaving/face.tpp
    smartSubPaving/pave_node.tpp

    maze/maze.tpp
    maze/room.tpp
    maze/door.tpp
    maze/sepmaze.tpp

    definition/domain.tpp
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

  add_library(invariant-core #SHARED
      ${SRC}
      ${HEADERS}
  )
  target_link_libraries(invariant-core
      ${LIBS}
      ${LIBS_NETCDF}
      ${LIBS_PPL}
      ${LIBS_GMP}
  )

    target_include_directories(invariant-core
        PUBLIC ${PPL_INCLUDES}
        PUBLIC ${GMP_INCLUDES}
        PUBLIC ${IBEX_INCLUDE_DIRS}
    )

  install(FILES ${HEADERS} DESTINATION "include/invariant-lib")
  install(TARGETS invariant-core DESTINATION lib)
