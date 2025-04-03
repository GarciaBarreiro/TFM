find_path(RANGE-V3_INCLUDE_DIR
    module.modulemap
    HINTS ${CMAKE_SOURCE_DIR}/lib/range-v3/include/)

message(STATUS "range-v3 include dir: ${RANGE-V3_INCLUDE_DIR}")

if(RANGE-V3_INCLUDE_DIR)
    set(RANGE-V3_FOUND ON)
endif(RANGE-V3_INCLUDE_DIR)

if(RANGE-V3_FOUND)
    if(NOT RANGE-V3_FIND_QUIETLY)
        message(STATUS "Findrange-v3lib: Found range-v3 header directory, ${RANGE-V3_INCLUDE_DIR}.")
    endif(NOT RANGE-V3_FIND_QUIETLY)
else(RANGE-V3_FOUND)
    if(RANGE-V3_FIND_REQUIRED)
        message(FATAL_ERROR "Findrange-v3lib: Could not find range-v3 header and/or library.")
    endif(RANGE-V3_FIND_REQUIRED)
endif(RANGE-V3_FOUND)
