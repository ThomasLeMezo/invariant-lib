# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================


# Sources
include_directories(definition)
include_directories(definition/netcdf)
include_directories(definition/dataSet)
include_directories(smartSubPaving)
include_directories(smartSubPaving/bisectionTree)
include_directories(maze)
include_directories(serialization)

  set(SRC
    serialization/ibex_serialization.cpp

    definition/dynamics.cpp
    definition/dynamics_function.cpp
    definition/dynamicsinclusionfunction.cpp
    definition/domain.cpp

    maze/room.cpp
    maze/door.cpp
    maze/maze.cpp
    smartSubPaving/face.cpp
    smartSubPaving/pave.cpp

    definition/dataSet/datasetfunction.cpp
    definition/dataSet/datasetnode.cpp
  )

  if(WITH_NETCDF)
    set(SRC ${SRC}
      definition/netcdf/nodecurrent.cpp
      definition/netcdf/previmer.cpp
      definition/netcdf/previmer3d.cpp
    )
  endif()

  set(HEADERS
    serialization/ibex_serialization.h

    definition/domain.h
    definition/domain.tpp
    definition/dynamics.h
    definition/dynamics_function.h
    definition/dynamicsinclusionfunction.h

    maze/maze.h
    maze/room.h
    maze/door.h
    maze/sepmaze.h

    smartSubPaving/smartSubPaving.h
    smartSubPaving/pave.h
    smartSubPaving/face.h
    smartSubPaving/pave_node.h
    smartSubPaving/bisectionTree/bisectiontree.h
    smartSubPaving/bisectionTree/bisectiontreeunion.h
    smartSubPaving/bisectionTree/bisectiontreeinter.h

    definition/dataSet/datasetnode.h
    definition/dataSet/datasetnode.tpp

    definition/dataSet/datasetfunction.h
    definition/dataSet/datasetvirtualnode.h

    smartSubPaving/smartSubPaving.tpp
    smartSubPaving/pave.tpp
    smartSubPaving/face.tpp
    smartSubPaving/pave_node.tpp
    smartSubPaving/bisectionTree/bisectiontree.tpp
    smartSubPaving/bisectionTree/bisectiontreeunion.tpp
    smartSubPaving/bisectionTree/bisectiontreeinter.tpp

    maze/maze.tpp
    maze/room.tpp
    maze/door.tpp
    maze/sepmaze.tpp
  )

  if(WITH_NETCDF)
    set(HEADERS ${HEADERS}
      definition/netcdf/nodecurrent.h
      definition/netcdf/previmer.h
      definition/netcdf/previmer3d.h
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
