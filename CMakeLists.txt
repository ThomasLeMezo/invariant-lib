# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

cmake_minimum_required(VERSION 2.8.7)

# Settings
    set(CMAKE_CXX_STANDARD 14)
    set(PACKAGE_NAME      invariant-lib)
    set(PACKAGE_VERSION "0.1")
    project(${PACKAGE_NAME})

    # Module to find ibex
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

    get_filename_component(INVARIANTLIB_BIN_DIR  "./bin"   ABSOLUTE)
    get_filename_component(INVARIANTLIB_LIB_DIR  "./lib"   ABSOLUTE)
    get_filename_component(INVARIANTLIB_DATA_DIR "./data"  ABSOLUTE)
    set(DATA_DIRECTORY "${INVARIANTLIB_DATA_DIR}" CACHE PATH "")
    

# debug/release options
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3")

# Options
    option(BUILD_TESTS "Build test" OFF)
    option(ONLY_PYTHON_BINDINGS "build library with python bindings only" OFF)

# Position Independent Code
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-fPIC SUPPORT_FPIC)
    if(SUPPORT_FPIC)
        set(CMAKE_CXX_FLAGS ${CMAKE_CSS_FLAGS} "-fPIC")
    endif(SUPPORT_FPIC)


# IBEX-lib package
    # set(IbexLib_USE_STATIC 1) # force ibex static linking
    FIND_PACKAGE(PkgConfig)
    set(PKG_CONFIG_PATH "${PKG_CONFIG_PATH}:${IBEX_ROOT}/share/pkgconfig")
    pkg_search_module(IBEX ibex)

    if(IBEX_FOUND)
        # https://cmake.org/Wiki/CMake:How_To_Find_Libraries
        include_directories(${IBEX_INCLUDE_DIRS})
        set(LIBS ${LIBS} ${IBEX_LIBRARIES})

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frounding-math")
    else()
        message(FATAL_ERROR "Unable to find IBEX-lib. You may need to set IBEX_ROOT ${IBEX_ROOT}")
    endif()

# OpenMP library
    #find_package(OpenMP)
    #if(OPENMP_FOUND)
    #    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    #    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    #endif()

# Sources
    add_subdirectory(src)

    IF(${CMAKE_BUILD_TYPE} MATCHES "DEBUG")
        include_directories(./build/build-debug/include)
        message("CMAKE_BUILD_TYPE = DEBUG")
    ELSEIF(${CMAKE_BUILD_TYPE} MATCHES "RELEASE")
        include_directories(./build/build-release/include)
        message("CMAKE_BUILD_TYPE = RELEASE")
    ELSE(${CMAKE_BUILD_TYPE} MATCHES "DEBUG")
        message(FATAL_ERRORO "not mode selected (CMAKE_BUILD_TYPE)")
    ENDIF(${CMAKE_BUILD_TYPE} MATCHES "DEBUG")


# Tests
    if(BUILD_TESTS)
        enable_testing(true)
        add_subdirectory(tests)
        include(CTest)
    endif()


# Examples
     add_subdirectory(examples)

# Uninstallation
    configure_file("${CMAKE_HOME_DIRECTORY}/cmake/Modules/cmake_uninstall.cmake.in" "cmake_uninstall.cmake" IMMEDIATE @ONLY)
    add_custom_target(uninstall "${CMAKE_COMMAND}" -P "cmake_uninstall.cmake")
