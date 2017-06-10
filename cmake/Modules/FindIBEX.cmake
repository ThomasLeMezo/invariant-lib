# - Try to find ImageMagick++
# Once done, this will define
#  Magick++_FOUND - system has Magick++
#  Magick++_INCLUDE_DIRS - the Magick++ include directories
#  Magick++_LIBRARIES - link these to use Magick++

find_package(LibFindMacros)

# Dependencies
libfind_package(IBEX ibex)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(IBEX_PKGCONF ibex)

# Include dir
find_path(IBEX_INCLUDE_DIR
  NAMES ibex.h
  HINTS ${IBEX_ROOT}
  PATH_SUFFIXES include include/ibex
)

# Finally the library itself
find_library(IBEX_LIBRARY
	NAMES libibex.a
	HINTS ${IBEX_ROOT}
	PATH_SUFFIXES lib
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(IBEX_PROCESS_INCLUDES IBEX_INCLUDE_DIR ibex_INCLUDE_DIRS)
set(IBEX_PROCESS_LIBS IBEX_LIBRARY ibex_LIBRARIES)

set(IBEX_LIBRARIES ${IBEX_LIBRARY})
set(IBEX_INCLUDE_DIRS ${IBEX_INCLUDE_DIR})

libfind_process(IBEX)