# Installation process handled by setup.py
#set(PYTHON_PACKAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/python_package")
#file(MAKE_DIRECTORY ${PYTHON_PACKAGE_DIR})
#execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/pyinvariant/" "${PYTHON_PACKAGE_DIR}/pyinvariant")
# execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/tests/" "${PYTHON_PACKAGE_DIR}/pyinvariant/tests")

# find_package(pybind11)

#configure_file(
#  ${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in
#  ${PYTHON_PACKAGE_DIR}/setup.py
#)

#set( pip_dir "${CMAKE_CURRENT_BINARY_DIR}/pip_package")

include_directories(.)
include_directories(../src/invariant/graph)
include_directories(../src/invariant/maze)
include_directories(../src/invariant/definition)
include_directories(../src/graphics)

pybind11_add_module(pyinvariant
    pyinvariant.cpp
)

target_link_libraries(pyinvariant PRIVATE
    invariant-core
)

#add_custom_command(TARGET invariant POST_BUILD
#	COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:invariant>" "${PYTHON_PACKAGE_DIR}/pyinvariant"
#)

# ADD_TEST(NAME python_module
#       COMMAND ${PYTHON_EXECUTABLE} -m unittest discover pyinvariant
#       WORKING_DIRECTORY ${PYTHON_PACKAGE_DIR}
# )

#install( DIRECTORY "${PYTHON_PACKAGE_DIR}/pyinvariant" DESTINATION "${PYTHON_INSTDIR}")

#add_custom_target(pip_package)
#add_custom_command(TARGET pip_package PRE_BUILD
#    COMMAND ${PYTHON_EXECUTABLE} ARGS setup.py bdist_wheel -d ${CMAKE_BINARY_DIR}
#    WORKING_DIRECTORY ${PYTHON_PACKAGE_DIR}
#)

#add_custom_target(install_python)
#add_custom_command(TARGET install_python PRE_BUILD
#    COMMAND pip install --upgrade .
#    WORKING_DIRECTORY ${PYTHON_PACKAGE_DIR}
#)

#add_dependencies(install_python invariant)
