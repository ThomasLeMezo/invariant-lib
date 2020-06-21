# Installation process handled by setup.py

set(PYTHON_PACKAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/python_package")
# file(MAKE_DIRECTORY ${PYTHON_PACKAGE_DIR})
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/pyinvariant/" "${PYTHON_PACKAGE_DIR}/pyinvariant")
# execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/tests/" "${PYTHON_PACKAGE_DIR}/pyinvariant/tests")

# find_package(pybind11)

file(WRITE "${PYTHON_PACKAGE_DIR}/empty.c" "// empty source file")

configure_file(
 ${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in
 ${PYTHON_PACKAGE_DIR}/setup.py
)

include_directories(.)
include_directories(../src/invariant/smartSubPaving)
include_directories(../src/invariant/smartSubPaving/booleanTree)
include_directories(../src/invariant/maze)
include_directories(../src/invariant/definition)
include_directories(../src/invariant/definition/netcdf)
include_directories(../src/invariant/definition/dataSet)
include_directories(../src/language)
include_directories(../src/graphics)

set(PYBIND11_CPP_STANDARD -std=c++11)
pybind11_add_module(pyinvariant
    pyinvariant.cpp
)

target_link_libraries(pyinvariant PRIVATE
    invariant-core
    invariant-graphics
    invariant-graphics-3D
    invariant-language
)

target_include_directories(pyinvariant
    PUBLIC ${IBEX_INCLUDE_DIRS}
    PUBLIC ${VTK_INCLUDE_DIRS}
    PUBLIC ${NETCDF_INCLUDE_DIRS}
    PUBLIC ${PROJ4_INCLUDE_DIR}
    PUBLIC ${SQLITE3_INCLUDE_DIRS}
)

# Copie des fichiers .so dans le module python
add_custom_command(TARGET pyinvariant POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:pyinvariant>" "${PYTHON_PACKAGE_DIR}/pyinvariant"
)

# ADD_TEST(NAME python_module
#       COMMAND ${PYTHON_EXECUTABLE} -m unittest discover pyinvariant
#       WORKING_DIRECTORY ${PYTHON_PACKAGE_DIR}
# )

install( DIRECTORY "${PYTHON_PACKAGE_DIR}/pyinvariant" DESTINATION "${PYTHON_INSTDIR}")

add_custom_target(pip_package)
add_custom_command(TARGET pip_package PRE_BUILD
   COMMAND ${PYTHON_EXECUTABLE} ARGS setup.py bdist_wheel -d ${CMAKE_BINARY_DIR}
   WORKING_DIRECTORY ${PYTHON_PACKAGE_DIR}
)

add_custom_target(install_python)
add_custom_command(TARGET install_python PRE_BUILD
    COMMAND pip install --upgrade .
    WORKING_DIRECTORY ${PYTHON_PACKAGE_DIR}
)

add_dependencies(install_python pyinvariant)

# --build-purelib

# ------------------------------
#add_custom_target(install_python)
#add_custom_command(TARGET install_python PRE_BUILD
#    COMMAND pip install --upgrade .
#    WORKING_DIRECTORY ${PYTHON_PACKAGE_DIR}
#)

