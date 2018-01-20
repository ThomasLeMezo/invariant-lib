# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================


include_directories(${CMAKE_SOURCE_DIR}/src/invariant/smartSubPaving)
include_directories(${CMAKE_SOURCE_DIR}/src/invariant/smartSubPaving/bisectionTree)
include_directories(${CMAKE_SOURCE_DIR}/src/invariant/maze)
include_directories(${CMAKE_SOURCE_DIR}/src/invariant/definition)
include_directories(${CMAKE_SOURCE_DIR}/src/tools/serialization)
include_directories(${CMAKE_SOURCE_DIR}/src/invariant/definition/netcdf)
include_directories(${CMAKE_SOURCE_DIR}/src/graphics)

add_subdirectory(cpp)
