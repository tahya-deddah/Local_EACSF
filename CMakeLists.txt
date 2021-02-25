Project(Local_EACSF)
cmake_minimum_required(VERSION 2.8.11)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)


# find Qt5 headers
if(UNIX AND NOT APPLE)
  find_package(Qt5 COMPONENTS Widgets Gui X11Extras Core DBus REQUIRED)
else(UNIX AND NOT APPLE)
  find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif(UNIX AND NOT APPLE)


include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})
set(QT_LIBRARIES ${Qt5Widgets_LIBRARIES})


find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(SlicerExecutionModel)
include(${SlicerExecutionModel_USE_FILE})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

add_subdirectory(src)
