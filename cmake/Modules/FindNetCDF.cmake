# - Try to find NETCDF
# Once done this will define
#  NETCDF_FOUND - System has ibex
#  NETCDF_INCLUDE_DIRS - The ibex include directories
#  NETCDF_LIBRARIES - The libraries needed to use ibex
#  NETCDF_DEFINITIONS - Compiler switches required for using ibex

find_package(PkgConfig)
#pkg_check_modules(PC_NETCDFLIB QUIET ibex)
message(STATUS "[NETCDF] NETCDF_ROOT ${NETCDF_ROOT}")

if(NetCDF_USE_STATIC)
  SET(CMAKE_FIND_LIBRARY_SUFFIXES .a)
endif()

set(NETCDF_DEFINITIONS ${PC_NETCDF_CFLAGS_OTHER})
find_path(NETCDF_INCLUDE_DIR netcdf
          HINTS ${NETCDF_ROOT}
          PATH_SUFFIXES include
          NO_DEFAULT_PATH)

find_library(NETCDF_LIBRARY NAMES netcdf_c++4
            HINTS ${NETCDF_ROOT}
            PATH_SUFFIXES lib
            NO_DEFAULT_PATH
						)

set(NETCDF_LIBRARIES ${NETCDF_LIBRARY})
set(NETCDF_INCLUDE_DIRS ${NETCDF_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set NETCDF_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(NETCDF  DEFAULT_MSG
                                  NETCDF_LIBRARY NETCDF_INCLUDE_DIR)

mark_as_advanced(NETCDF_INCLUDE_DIR NETCDF_LIBRARY )
