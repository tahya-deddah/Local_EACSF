cmake_minimum_required(VERSION 2.8.7)
cmake_policy(SET CMP0071  NEW)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#project(Local_EACSF)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
set_target_properties(Qt5::Core PROPERTIES MAP_IMPORTED_CONFIG_COVERAGE "RELEASE")


SET(APP_NAME Local_EACSF)


set(${APP_NAME}_moc csfwindow.h)
set(${APP_NAME}_ui csfwindow.ui )
set(${APP_NAME}_rcc Local_EACSF_Resources.qrc)

set_property(SOURCE csfwindow.h PROPERTY SKIP_AUTOMOC ON)


qt5_wrap_cpp(${APP_NAME}_generated_moc ${${APP_NAME}_moc})
qt5_wrap_ui(${APP_NAME}_generated_ui ${${APP_NAME}_ui})
qt5_add_resources(${APP_NAME}_generated_rcc ${${APP_NAME}_rcc})



list(APPEND ${APP_NAME}_src
	    csfwindow.cpp
	    extexecutableswidget.cxx
	    csfscripts.cpp
	    ${${APP_NAME}_generated_moc}
	    ${${APP_NAME}_generated_ui}
	    ${${APP_NAME}_generated_rcc}
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})


include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})
set(QT_LIBRARIES ${Qt5Widgets_LIBRARIES})


find_package(SlicerExecutionModel)
include(${SlicerExecutionModel_USE_FILE})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})



SET(${APP_NAME}_LIBRARIES ${ITK_LIBRARIES} ${QT_LIBRARIES})

SEMMacroBuildCLI(
NAME ${APP_NAME}
EXECUTABLE_ONLY
ADDITIONAL_SRCS ${${APP_NAME}_src}
TARGET_LIBRARIES ${${APP_NAME}_LIBRARIES}
RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
INSTALL_RUNTIME_DESTINATION ${INSTALL_RUNTIME_DESTINATION}
INSTALL_LIBRARY_DESTINATION ${INSTALL_LIBRARY_DESTINATION}
INSTALL_ARCHIVE_DESTINATION ${INSTALL_ARCHIVE_DESTINATION}

)






