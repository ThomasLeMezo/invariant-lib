# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================


# Headers stored into an include directory

  set(HEADERS_DIR ${CMAKE_CURRENT_BINARY_DIR}/../include)
  file(MAKE_DIRECTORY ${HEADERS_DIR})

# Sources

  add_subdirectory(invariant)
  add_subdirectory(graphics)
  add_subdirectory(language)
  add_subdirectory(exp)
