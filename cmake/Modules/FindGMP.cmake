# Try to find the GMP librairies
#  GMP_FOUND - system has GMP lib
#  GMP_INCLUDE_DIR - the GMP include directory
#  GMP_LIBRARIES - Libraries needed to use GMP

# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (NOT GMP_ROOT_DIR)
  set (GMP_ROOT_DIR ${GMP_ROOT})
endif (NOT GMP_ROOT_DIR)

find_path(GMP_INCLUDES
	NAMES gmp.h gmpxx.h
        HINTS ${GMP_ROOT_DIR}
        PATH_SUFFIXES include
        NO_DEFAULT_PATH
)
message (STATUS "[GMP] ROOT/LIBRARIES/LIBRARIES_C ${GMP_ROOT_DIR} ${GMP_LIBRARIES_CXX} ${GMP_LIBRARIES_C}")

find_library(GMP_LIBRARIES_C
	NAMES gmp
        HINTS ${GMP_ROOT_DIR}
        PATH_SUFFIXES lib
        NO_DEFAULT_PATH
)

find_library(GMP_LIBRARIES_CXX
	NAMES gmpxx
        HINTS ${GMP_ROOT_DIR}
        PATH_SUFFIXES lib
        NO_DEFAULT_PATH
)

set(GMP_LIBRARIES "${GMP_LIBRARIES_CXX};${GMP_LIBRARIES_C}")

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GMP DEFAULT_MSG GMP_INCLUDES GMP_LIBRARIES)

if (GMP_FOUND)
  if (NOT GMP_FIND_QUIETLY)
    message (STATUS "[GMP] Found components for GMP")
    message (STATUS "[GMP] GMP_ROOT_DIR  = ${GMP_ROOT_DIR}")
    message (STATUS "[GMP] GMP_INCLUDES  = ${GMP_INCLUDES}")
    message (STATUS "[GMP] GMP_LIBRARIES = ${GMP_LIBRARIES}")
  endif (NOT GMP_FIND_QUIETLY)
else (GMP_FOUND)
  if (GMP_FIND_REQUIRED)
    message (FATAL_ERROR "[GMP] Could not find GMP!")
  endif (GMP_FIND_REQUIRED)
endif (GMP_FOUND)

mark_as_advanced(GMP_INCLUDE_DIR GMP_LIBRARIES)
