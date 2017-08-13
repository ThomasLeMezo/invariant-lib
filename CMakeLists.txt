# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

cmake_minimum_required(VERSION 3.5)

# Settings
    set(CMAKE_CXX_STANDARD 14)
    set(PACKAGE_NAME      invariant-lib)
    set(PACKAGE_VERSION "0.1")
    project(${PACKAGE_NAME})

    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/)

# debug/release options
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -Wno-reorder")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3 -Wno-reorder")

# Options
    option(BUILD_TESTS "Build test" OFF)
    option(WITH_PYTHON "Build python module" ON)

    if(NOT WIN32)
        set(CMAKE_CXX_FLAGS " -fPIC ${CMAKE_CXX_FLAGS}")
    endif()

    FIND_PACKAGE(PkgConfig)

# IBEX-lib package
    set(IbexLib_USE_STATIC 1)
    # set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${IBEX_ROOT}/share/pkgconfig/")
    # message(STATUS "[IBEX] PKG_CONFIG_PATH = $ENV{PKG_CONFIG_PATH}")
    # pkg_search_module(IBEX ibex)

    FIND_PACKAGE(IbexLib REQUIRED)
    if(IBEX_FOUND)
        # https://cmake.org/Wiki/CMake:How_To_Find_Libraries
        INCLUDE_DIRECTORIES(${IBEX_INCLUDE_DIRS})
        set(LIBS ${LIBS} ${IBEX_LIBRARIES})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frounding-math")

        # link_directories("${IBEX_ROOT}/lib" "${IBEX_ROOT}/lib/ibex/3rd")

        message(STATUS "[IBEX] IBEX_LIBRARIES = ${IBEX_LIBRARIES}")
        message(STATUS "[IBEX] IBEX_INCLUDE_DIRS = ${IBEX_INCLUDE_DIRS}")
        message(STATUS "[IBEX] IBEX_DEFINITIONS = ${IBEX_DEFINITIONS}")
        message(STATUS "[IBEX] CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
    else()
        message(FATAL_ERROR "Unable to find IBEX-lib. You may need to set IBEX_ROOT ${IBEX_ROOT}")
    endif()


# Graphiz
    pkg_search_module(LIBGVC libgvc)
    if(LIBGVC_FOUND)
        include_directories(${LIBGVC_INCLUDE_DIRS})
        set(LIBS_GRAPHIZ ${LIBS_GRAPHIZ} ${LIBGVC_LIBRARIES})
        message(STATUS "[LIBGVC] OK")
    else()
        message(WARNING "Unable to find LIBGVC")
    endif()

# OpenMP library
    find_package(OpenMP)
    if(OPENMP_FOUND)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        message(STATUS "[OPENMP] OK")
    else()
        message(WARNING "-- [OPENMP] NO")
    endif()

# VTK
    set(VTK_DIR "${VTK_ROOT}")
    find_package(VTK REQUIRED)
    if(VTK_FOUND)
        set(LIBS_VTK ${LIBS_VTK} ${VTK_LIBRARIES})
        message(STATUS "[VTK] OK")
    else()
        message(WARNING "-- [VTK] NO")
    endif()

# Netcdf
    pkg_search_module(NETCDF netcdf-cxx4)
    if(NETCDF_FOUND)
        include_directories(${NETCDF_INCLUDE_DIRS})
        set(LIBS_NETCDF ${LIBS_NETCDF} ${NETCDF_LIBRARIES})
        message(STATUS "[NETCDF] OK")
    else()
        message(WARNING "-- [NETCDF] NO")
    endif()

# Sources
    message(STATUS "Include directories")
    get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
    foreach(dir ${dirs})
      message(STATUS "dir='${dir}'")
    endforeach()

    message(STATUS "Link directories")
    get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
    foreach(dir ${dirs})
      message(STATUS "dir='${dir}'")
    endforeach()

    add_subdirectory(src)

# Pybind11
    if(WITH_PYTHON)
        message(STATUS "[PYTHON] ON")
#        set(PYBIND11_CPP_STANDARD -std=c++14)
        add_subdirectory(pybind11)
        add_subdirectory(python)
    else()
        message(STATUS "[PYTHON] OFF")
    endif()

# Tests
    if(BUILD_TESTS)
        message(STATUS "[TESTS] ON")
        enable_testing(true)
        add_subdirectory(tests)
        include(CTest)
    else()
        message(STATUS "[TESTS] OFF")
    endif()

# Examples
     add_subdirectory(examples)

# Uninstallation
    configure_file("${CMAKE_HOME_DIRECTORY}/cmake/Modules/cmake_uninstall.cmake.in" "cmake_uninstall.cmake" IMMEDIATE @ONLY)
    add_custom_target(uninstall "${CMAKE_COMMAND}" -P "cmake_uninstall.cmake")
