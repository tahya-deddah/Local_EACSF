#cmake_minimum_required(VERSION 2.8.7)
cmake_policy(SET CMP0071  NEW)
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED True)


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

find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

SEMMacroBuildCLI(
	NAME ${APP_NAME}
	EXECUTABLE_ONLY
	ADDITIONAL_SRCS ${${APP_NAME}_src}
	TARGET_LIBRARIES ${QT_LIBRARIES}
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
	ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
	INSTALL_RUNTIME_DESTINATION ${INSTALL_RUNTIME_DESTINATION}
	INSTALL_LIBRARY_DESTINATION ${INSTALL_LIBRARY_DESTINATION}
	INSTALL_ARCHIVE_DESTINATION ${INSTALL_ARCHIVE_DESTINATION}
)






