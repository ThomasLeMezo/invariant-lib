# ==================================================================
#  invariant-lib - cmake configuration file
# ==================================================================

cmake_minimum_required(VERSION 3.2.2)

# Settings
set(CMAKE_CXX_STANDARD 11)
set(PACKAGE_NAME      invariant-lib)
set(PACKAGE_VERSION "0.3b2")
project(${PACKAGE_NAME})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/)

# debug/release options
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -Wno-reorder -Wno-unknown-pragmas -Wno-unused-variable")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3 -Wno-reorder -Wno-unknown-pragmas -Wno-unused-variable")

# Options
option(WITH_EXAMPLES "Build examples" ON)
option(BUILD_TESTS "Build test" OFF)
option(WITH_PYTHON "Build python module" ON)
option(WITH_3D "Build 3D src and examples" ON)
option(WITH_GRAPHIZ "Build graphiz src and examples" OFF)
option(WITH_NETCDF "Build netcdf src and examples" ON)
option(WITH_PYIBEX_VERSION "Build with ibex version of pyibex" ON)
option(WITH_PPL "Build with ppl" ON)
option(WITH_PROJ4 "Build with Proj4 lib" ON)

if(NOT WIN32)
 set(CMAKE_CXX_FLAGS " -fPIC ${CMAKE_CXX_FLAGS}")
endif()

FIND_PACKAGE(PkgConfig)

#####################################################
# OpenMP library
find_package(OpenMP)
if(OPENMP_FOUND)
 set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
 message(STATUS "[OPENMP] OK")
else()
 message(FATAL_ERROR "[OPENMP] NO")
endif()

#####################################################
############## IBEX-lib package #####################
set(IbexLib_USE_STATIC 1)
FIND_PACKAGE(IbexLib REQUIRED)
if(IBEX_FOUND)
 set(LIBS ${LIBS} ${IBEX_LIBRARIES})
 include_directories(${IBEX_INCLUDE_DIRS})

 message(STATUS "[IBEX] IBEX_LIBRARIES = ${IBEX_LIBRARIES}")
 message(STATUS "[IBEX] IBEX_INCLUDE_DIRS = ${IBEX_INCLUDE_DIRS}")
 message(STATUS "[IBEX] IBEX_DEFINITIONS = ${IBEX_DEFINITIONS}")
 message(STATUS "[IBEX] CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
else()
 message(FATAL_ERROR "Unable to find IBEX-lib. You may need to set IBEX_ROOT ${IBEX_ROOT}")
endif()

#####################################################
############## Graphiz ##############################
if(WITH_GRAPHIZ)
 pkg_search_module(LIBGVC libgvc)
 if(LIBGVC_FOUND)
  include_directories(${LIBGVC_INCLUDE_DIRS})
  set(LIBS_GRAPHIZ ${LIBS_GRAPHIZ} ${LIBGVC_LIBRARIES})
  message(STATUS "[LIBGVC] OK")
 else()
  message(FATAL_ERROR "[LIBGVC] Unable to find LIBGVC")
 endif()
endif()

#####################################################
############## VTK ##################################
if(WITH_3D)
 find_package(VTK REQUIRED
  PATHS ${VTK_ROOT}
  NO_DEFAULT_PATH
  )
 if(VTK_FOUND)
  INCLUDE(${VTK_USE_FILE})
  set(LIBS_VTK ${LIBS_VTK} ${VTK_LIBRARIES})
  include_directories(${VTK_INCLUDE_DIRS})
  #link_directories(${VTK_RUNTIME_LIBRARY_DIRS})
  message (STATUS "[VTK] VTK_DIR  = ${VTK_DIR}")
  message (STATUS "[VTK] VTK_BUILD_VERSION  = ${VTK_BUILD_VERSION}")
  message (STATUS "[VTK] VTK_INCLUDE_DIRS  = ${VTK_INCLUDE_DIRS}")
  message (STATUS "[VTK] VTK_RUNTIME_LIBRARY_DIRS = ${VTK_RUNTIME_LIBRARY_DIRS}")
  message(STATUS "[VTK] OK")
 else()
  message(FATAL_ERROR "[VTK] NO")
 endif()
endif()

#####################################################
# Netcdf
if(WITH_NETCDF)
 set(NetCDF_USE_STATIC 1)
 find_package(NetCDF REQUIRED)
 if(NETCDF_FOUND)
  include_directories(${NETCDF_INCLUDE_DIRS})
  set(LIBS_NETCDF ${LIBS_NETCDF} ${NETCDF_LIBRARIES})
  set(LIBS_NETCDF ${LIBS_NETCDF} netcdf)
  message(STATUS "[NETCDF] OK")
  message (STATUS "[NETCDF] NETCDF_INCLUDE_DIRS  = ${NETCDF_INCLUDE_DIRS}")
  message (STATUS "[NETCDF] NETCDF_LIBRARY = ${NETCDF_LIBRARY}")
 else()
  message(FATAL_ERROR "[NETCDF] NO")
 endif()
endif()

#####################################################
# Proj4
if(WITH_PROJ4)
	set(Proj4_USE_STATIC 1)
	find_package(Proj4 REQUIRED
#		PATHS ${PROJ4_ROOT}
#		NO_DEFAULT_PATH
		)
 if(PROJ4_FOUND)
  include_directories(${PROJ4_INCLUDE_DIR})
  set(LIBS_PROJ4 ${LIBS_PROJ4} ${PROJ4_LIBRARIES})
  message (STATUS "[PROJ4] PROJ4_INCLUDE_DIRS  = ${PROJ4_INCLUDE_DIR}")
  message (STATUS "[PROJ4] LIBS_PROJ4 = ${LIBS_PROJ4}")
  message (STATUS "[PROJ4] PJ_VERSIONSTR = ${PJ_VERSIONSTR}")
  message(STATUS "[PROJ4] OK")
 else()
  message(FATAL_ERROR "[PROJ4] NO")
 endif()

	find_package (SQLite3 REQUIRED)
	if (SQLITE3_FOUND)
			include_directories(${SQLITE3_INCLUDE_DIRS})
			set(LIBS_PROJ4 ${LIBS_PROJ4} ${SQLITE3_LIBRARIES})
			message (STATUS "[SQLITE3] SQLITE3_INCLUDE_DIRS  = ${SQLITE3_INCLUDE_DIRS}")
			message (STATUS "[SQLITE3] SQLITE3_LIBRARIES = ${SQLITE3_LIBRARIES}")
			message(STATUS "[SQLITE3] OK")
	endif (SQLITE3_FOUND)

endif()

#####################################################
# PPL
if(WITH_PPL)
 find_package(GMP REQUIRED)
 if(GMP_FOUND)
  set(LIBS_GMP ${LIBS_GMP} ${GMP_LIBRARIES})
  message(STATUS "[GMP] OK")

  set(CMAKE_C_FLAGS "-fPIC ${CMAKE_C_FLAGS} -lgmpxx -lgmp")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lgmpxx -lgmp")
  message(STATUS "[GMP] CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")
  message(STATUS "[GMP] CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
 else()
  message(FATAL_ERROR "[GMP] NO")
 endif()

 find_package(PPL REQUIRED)
 if(PPL_FOUND)
  set(LIBS_PPL ${LIBS_PPL} ${PPL_LIBRARIES})
  message(STATUS "[PPL] OK (${PPL_ROOT_DIR})")
 else()
  message(FATAL_ERROR "[PPL] NO")
 endif()
endif()

#####################################################
# Sources
add_subdirectory(src)

#####################################################
# Pybind11
if(WITH_PYTHON)
 message(STATUS "[PYBIND11] ON")
 #        set(PYBIND11_CPP_STANDARD -std=c++11)
 add_subdirectory(pybind11)
 add_subdirectory(python)
else()
 message(STATUS "[PYBIND11] OFF")
endif()

#####################################################
# Tests
if(BUILD_TESTS)
 message(STATUS "[TESTS] ON")
 enable_testing(true)
 add_subdirectory(tests)
 include(CTest)
else()
 message(STATUS "[TESTS] OFF")
endif()

#####################################################
# Examples
if(WITH_EXAMPLES)
 add_subdirectory(examples)
endif()

#####################################################
# Uninstallation
configure_file("${CMAKE_HOME_DIRECTORY}/cmake/Modules/cmake_uninstall.cmake.in" "cmake_uninstall.cmake" IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "cmake_uninstall.cmake")
