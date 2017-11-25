# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

include_directories(../invariant/smartSubPaving)
include_directories(../invariant/maze)
include_directories(../invariant/definition)
include_directories(../invariant/definition/netcdf)
include_directories(../invariant/serialization)
include_directories(vibes)

if(WITH_3D)
    include(${VTK_USE_FILE})

    set(SRC_3D
        vtkMaze3D.cpp
    )
    set(HEADERS_3D
        vtkMaze3D.h
    )

    add_library(invariant-graphics-3D
        ${SRC_3D}
        ${HEADERS_3D}
    )
    target_link_libraries(invariant-graphics-3D
        ${LIBS}
        ${LIBS_VTK}
        invariant-core
    )

    install(FILES ${HEADERS_3D} DESTINATION "include/invariant-lib")
    install(TARGETS invariant-graphics-3D DESTINATION lib)
endif()

if(WITH_GRAPHIZ)
    set(SRC_graphiz
        graphiz_graph.cpp
    )

    set(HEADERS_graphiz
        graphiz_graph.h
    )

    add_library(invariant-graphics-graphiz
        ${SRC_graphiz}
        ${HEADERS_graphiz}
    )
    target_link_libraries(invariant-graphics-graphiz
        ${LIBS}
        ${LIBS_GRAPHIZ}
    )

    install(FILES ${HEADERS_graphiz} DESTINATION "include/invariant-lib")
    install(TARGETS invariant-graphics-graphiz DESTINATION lib)
endif()

  set(SRC
    vibes/vibes.cpp
    VibesFigure.cpp
    vibesMaze.cpp
  )

  set(HEADERS
    vibes/vibes.h

    VibesFigure.h
    vibesMaze.h

  )

  file(COPY ${HEADERS} DESTINATION ${HEADERS_DIR})

  add_library(invariant-graphics
      ${SRC}
      ${HEADERS}
  )
  target_link_libraries(invariant-graphics
      ${LIBS}
  )

  install(FILES ${HEADERS} DESTINATION "include/invariant-lib")
  install(TARGETS invariant-graphics DESTINATION lib)


