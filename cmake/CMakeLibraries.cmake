# ---  L I B R A R I E S  --- #
# --------------------------- #

# Add module directory to the include path.
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake/modules")

# Add lib/ folder to the list of folder where CMake looks for packages
set(LIB_FOLDER "${CMAKE_SOURCE_DIR}/lib")
list(APPEND CMAKE_MODULE_PATH ${LIB_FOLDER})

# OpenMP
find_package(OpenMP REQUIRED)
if (OPENMP_CXX_FOUND)
    message(STATUS "OpenMP found and to be linked")
else ()
    message(SEND_ERROR "Could not find OpenMP")
endif ()

# MPI
find_package(MPI REQUIRED)
if (MPI_CXX_FOUND)
    message(STATUS "MPI found and to be linked")
else()
    message(SEND_ERROR "Could not find MPI")
endif()

# Lapack
find_package(LAPACK REQUIRED)
if (LAPACK_FOUND)
    message(STATUS "Lapack found and to be linked")
else()
    message(SEND_ERROR "Could not find Lapack")
endif()

# Blas
find_package(BLAS REQUIRED)
if (BLAS_FOUND)
    message(STATUS "BLAS found and to be linked")
else()
    message(SEND_ERROR "Could not find BLAS")
endif()

# Armadillo
find_package(Armadillo REQUIRED)
if (TARGET armadillo::armadillo)
    message(STATUS "Dependency armadillo::armadillo found")
elseif (${ARMADILLO_FOUND})
    include_directories(${ARMADILLO_INCLUDE_DIR})
    message(STATUS "Armadillo include: " ${ARMADILLO_INCLUDE_DIR})
    message(STATUS "Armadillo libraries: " ${ARMADILLO_LIBRARIES})
else ()
    message(SEND_ERROR "Could not find armadillo::armadillo")
endif ()

# Eigen3
find_package(Eigen3 REQUIRED)
if (TARGET Eigen3::Eigen)
    message(STATUS "Dependency Eigen3::Eigen found")
elseif (${EIGEN3_FOUND})
    include_directories(${EIGEN3_INCLUDE_DIR})
    message(STATUS "Eigen include: ${EIGEN3_INCLUDE_DIR}")
else ()
    message(SEND_ERROR "Could not find Eigen3")
endif ()

# LASlib
find_package(LASLIB REQUIRED)
if (${LASLIB_FOUND})
    include_directories(${LASLIB_INCLUDE_DIR} ${LASZIP_INCLUDE_DIR})
    message(STATUS "LASlib include: ${LASLIB_INCLUDE_DIR} ${LASZIP_INCLUDE_DIR}")
else ()
    message(SEND_ERROR "Could not find LASLIB")
endif ()

# MathGeoLib
find_package(MATHGEOLIB REQUIRED)
if (${MATHGEOLIB_FOUND})
    include_directories(${MATHGEOLIB_INCLUDE_DIR})
    message(STATUS "MathGeoLib include: ${MATHGEOLIB_INCLUDE_DIR}")
else ()
    message(SEND_ERROR "Could not find MathGeoLib")
endif ()

# Range v3
find_package(range-v3 REQUIRED)
if (TARGET range-v3::range-v3)
    message(STATUS "Dependency range-v3::range-v3 found")
elseif (${RANGE-V3_FOUND})
    include_directories(${RANGE-V3_INCLUDE_DIR})
    message(STATUS "range-v3 include: ${RANGE-V3_INCLUDE_DIR}")
else ()
    message(SEND_ERROR "Could not find range-v3")
endif ()
