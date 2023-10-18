# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================


# Sources

set(SRC
  diff_expbox.cpp
  expbox_util.cpp
)

set(HEADERS
  diff_expbox.h
  expbox_util.h
)

file(COPY ${HEADERS} DESTINATION ${HEADERS_DIR})

add_library(explib #SHARED
    ${SRC}
    ${HEADERS}
)

target_link_libraries(explib
    ${LIBS}
)

target_include_directories(explib
    PUBLIC ${IBEX_INCLUDE_DIRS}
)

install(FILES ${HEADERS} DESTINATION "include/exp-lib")
install(TARGETS explib DESTINATION lib)
