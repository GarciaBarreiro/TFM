# This module defines the following uncached variables:
#  MATHGEOLIB_FOUND, if false, do not try to use MathGeoLib.
#  MATHGEOLIB_INCLUDE_DIR, where to find MathGeoLib.h.
#  MATHGEOLIB_LIBRARIES, the libraries to link against to use the MathGeoLib library.
#  MATHGEOLIB_LIBRARY_DIRS, the directory where the MathGeoLib library is found.

find_path(MATHGEOLIB_INCLUDE_DIR
        MathGeoLib.h
        HINTS ${CMAKE_SOURCE_DIR}/lib/MathGeoLib/src)

if (MATHGEOLIB_INCLUDE_DIR)
    find_library(MATHGEOLIB_LIBRARY
            NAMES MathGeoLib
            HINTS ${CMAKE_SOURCE_DIR}/lib/MathGeoLib)
    if (MATHGEOLIB_LIBRARY)
        # Set uncached variables as per standard.
        set(MATHGEOLIB_FOUND ON)
        set(MATHGEOLIB_LIBRARIES ${MATHGEOLIB_LIBRARY})
        get_filename_component(MATHGEOLIB_LIBRARY_DIRS ${MATHGEOLIB_LIBRARY} PATH)
    endif (MATHGEOLIB_LIBRARY)
endif (MATHGEOLIB_INCLUDE_DIR)

if (MATHGEOLIB_FOUND)
    if (NOT MATHGEOLIB_FIND_QUIETLY)
        message(STATUS "FindMATHGEOLIB: Found MATHGEOLIB header directory, ${MATHGEOLIB_INCLUDE_DIR}, and library, ${MATHGEOLIB_LIBRARIES}.")
    endif (NOT MATHGEOLIB_FIND_QUIETLY)
else (MATHGEOLIB_FOUND)
    if (MATHGEOLIB_FIND_REQUIRED)
        message(FATAL_ERROR "FindMATHGEOLIB: Could not find MATHGEOLIB header and/or library.")
    endif (MATHGEOLIB_FIND_REQUIRED)
endif (MATHGEOLIB_FOUND)