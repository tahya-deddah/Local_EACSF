if( NOT EXTERNAL_SOURCE_DIRECTORY )
  set( EXTERNAL_SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ExternalSources )
endif()
if( NOT EXTERNAL_BINARY_DIRECTORY )
  set( EXTERNAL_BINARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
endif()

get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED ShapePopulationViewer_SOURCE_DIR AND NOT EXISTS ${ShapePopulationViewer_SOURCE_DIR})
  message(FATAL_ERROR "ShapePopulationViewer_SOURCE_DIR variable is defined but corresponds to non-existing directory")
endif()

set(extProjName ShapePopulationViewer) #The find_package known name
set(proj      ShapePopulationViewer) #This local name
set(${extProjName}_REQUIRED_VERSION "")  #If a required version is necessary, then set this, else leave blank

if(NOT ( DEFINED "USE_SYSTEM_${extProjName}" AND "${USE_SYSTEM_${extProjName}}" ) )

  # Set CMake OSX variable to pass down the external project
  set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
  if(APPLE)
    list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()
 
  ExternalProject_Add(${proj}
    GIT_REPOSITORY "https://github.com/NIRALUser/ShapePopulationViewer.git"
    SOURCE_DIR ${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    "${cmakeversion_external_update}"
    CMAKE_ARGS
      -DShapePopulationViewer_SUPERBUILD:BOOL=ON
      -DCMAKE_C_FLAGS:STRING=-fPIC
      -DCMAKE_CXX_FLAGS:STRING=-fPIC
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      -DCOMPILE_COMMANDLINE:BOOL=ON
      #-DCOMPILE_SLICER4COMMANDLINE:BOOL=OFF
      -DCMAKE_INSTALL_PREFIX:PATH=${EXTERNAL_BINARY_DIRECTORY}/${proj}-install
      -DINSTALL_RUNTIME_DESTINATION=bin
      ${${proj}_CMAKE_OPTIONS}   
    INSTALL_COMMAND ""
    )
  set(${proj}_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(${proj}CommonLib_DIR    ${CMAKE_BINARY_DIR}/${proj}-build/ShapePopulationViewer-build/ShapePopulationViewerCommonLib)
  set(${proj}_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
else()
endif()
