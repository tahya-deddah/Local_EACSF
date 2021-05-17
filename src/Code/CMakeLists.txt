find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
SET(${APP_NAME}_LIBRARIES ${ITK_LIBRARIES})



add_subdirectory(CreateOuterImage)
add_subdirectory(CreateOuterSurface)
add_subdirectory(EditPolyData)
add_subdirectory(EstimateCortexStreamlinesDensity)
add_subdirectory(klaplace)
add_subdirectory(AddScalarstoPolyData)
add_subdirectory(HeatKernelSmoothing)
add_subdirectory(ComputeCSFVolume)
add_subdirectory(ComputeAverageMesh)
