find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
SET(${APP_NAME}_LIBRARIES ${VTK_LIBRARIES})


find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
SET(${APP_NAME}_LIBRARIES ${ITK_LIBRARIES} ${VTK_LIBRARIES})


add_subdirectory(app)
add_subdirectory(Code)
