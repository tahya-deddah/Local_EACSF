PROJECT(EACSFKlaplace)

cmake_minimum_required(VERSION 2.8)
#cmake_policy(SET CMP0071  NEW)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

FIND_PACKAGE(VXL REQUIRED)
INCLUDE(${VXL_CMAKE_DIR}/UseVXL.cmake)

find_package(Qt5 COMPONENTS X11Extras REQUIRED)

#INCLUDE_DIRECTORIES(/opt/local/include/vxl/core /opt/local/include/vxl/vcl)
#LINK_DIRECTORIES(/opt/local/lib/vxl)

ADD_EXECUTABLE(EACSFKlaplace klaplace.cpp klaplace.h kgeometry.cpp kgeometry.h kstreamtracer.cpp kstreamtracer.h kvolume.cpp kvolume.h kvtkutils.cpp kvtkutils.h vtkio.cpp vtkio.h piOptions.h piOptions.cpp csv_parser.cpp csv_parser.h SimpleOpt.h)
TARGET_LINK_LIBRARIES(EACSFKlaplace ${VTK_LIBRARIES} vnl)

install (
	TARGETS EACSFKlaplace
    ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DESTINATION}
    LIBRARY DESTINATION ${INSTALL_LIBRARY_DESTINATION}
    RUNTIME DESTINATION ${INSTALL_RUNTIME_DESTINATION} 
    )



