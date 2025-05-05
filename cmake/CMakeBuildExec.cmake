# ---  B U I L D I N G  --- #
# ------------------------- #

# Executable
add_executable(${PROJECT_NAME} ${sources})

# Linking libraries
if (TARGET OpenMP::OpenMP_CXX)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            OpenMP::OpenMP_CXX)
endif ()

if (TARGET MPI::MPI_CXX)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            MPI::MPI_CXX)
endif()

if (TARGET LAPACK::LAPACK)
        target_link_libraries(${PROJECT_NAME}
            PRIVATE
            LAPACK::LAPACK)
endif()

if (TARGET BLAS::BLAS)
        target_link_libraries(${PROJECT_NAME}
            PRIVATE
            BLAS::BLAS)
endif()

if (TARGET armadillo::armadillo AND
        ARMADILLO_VERSION_MAJOR GREATER 13)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            armadillo::armadillo)
else ()
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            ${ARMADILLO_LIBRARIES})
endif ()

if (TARGET Eigen3::Eigen)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            Eigen3::Eigen)
else ()
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            ${EIGEN_LIBRARIES})
endif ()

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ${LASLIB_LIBRARIES})

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ${MATHGEOLIB_LIBRARIES})

if (TARGET range-v3::range-v3)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            range-v3::range-v3)
else ()
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            ${RANGE-V3_LIBRARIES})
endif ()
