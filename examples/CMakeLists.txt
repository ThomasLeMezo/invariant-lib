# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================


include_directories(${CMAKE_SOURCE_DIR}/src/invariant/graph)
include_directories(${CMAKE_SOURCE_DIR}/src/invariant/maze)
include_directories(${CMAKE_SOURCE_DIR}/src/tools/)
include_directories(${CMAKE_SOURCE_DIR}/src/graphics/)

add_subdirectory(cpp)
