# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

#################################  3D (VTK) #################################
if(WITH_LANGUAGE)

  include_directories(../invariant/smartSubPaving)
  include_directories(../invariant/smartSubPaving/booleanTree)
  include_directories(../invariant/maze)
  include_directories(../invariant/definition)
  include_directories(../invariant/definition/netcdf)
  include_directories(../invariant/serialization)
  include_directories(../graphics/vibes)
  include_directories(../graphics)
  include_directories(../exp)


    set(SRC_LANGUAGE
        language.cpp
    )
    set(HEADERS_LANGUAGE
        language.h
    )

    add_library(invariant-language
        ${SRC_LANGUAGE}
        ${HEADERS_LANGUAGE}
    )

    target_link_libraries(invariant-language
        invariant-core
        invariant-graphics
				invariant-graphics-3D
    )

    install(FILES ${HEADERS_LANGUAGE} DESTINATION "include/invariant-lib")
    install(TARGETS invariant-language DESTINATION lib)
endif()


