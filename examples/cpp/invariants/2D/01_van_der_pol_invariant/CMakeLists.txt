# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

# Sources

set(SRC
 main.cpp
)

set(HEADERS

)

add_executable(2D_invariant_van_der_pol ${SRC})

target_link_libraries(2D_invariant_van_der_pol
  invariant-core
  invariant-graphics
  ${LIBS}
)