cmake_minimum_required (VERSION 2.8.11)
project (Local_EACSF)

#Qt example------------------------------------

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/lib)


if(NOT INSTALL_RUNTIME_DESTINATION)
	set(INSTALL_RUNTIME_DESTINATION bin)
endif(NOT INSTALL_RUNTIME_DESTINATION)

if(NOT INSTALL_LIBRARY_DESTINATION)
	set(INSTALL_LIBRARY_DESTINATION lib)
endif(NOT INSTALL_LIBRARY_DESTINATION)

if(NOT INSTALL_ARCHIVE_DESTINATION)
	set(INSTALL_ARCHIVE_DESTINATION lib)
endif(NOT INSTALL_ARCHIVE_DESTINATION)


# Find the QtWidgets library

# find Qt5 headers
if(UNIX AND NOT APPLE)
  find_package(Qt5 COMPONENTS Widgets Gui X11Extras DBus REQUIRED)
else(UNIX AND NOT APPLE)
  find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif(UNIX AND NOT APPLE)

include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})

set(QT_LIBRARIES ${Qt5Widgets_LIBRARIES})

# get Local_EACSF info
FILE(READ src/app/Local_EACSF.xml var)

string(REGEX MATCH "<title>.*</title>" ext "${var}")
string(REPLACE "<title>" "" title ${ext})
string(REPLACE "</title>" "" title ${title})
add_definitions(-DLocal_EACSF_TITLE="${title}")

string(REGEX MATCH "<version>.*</version>" ext "${var}")
string(REPLACE "<version>" "" version ${ext})
string(REPLACE "</version>" "" version ${version})
add_definitions(-DLocal_EACSF_VERSION="${version}")

string(REGEX MATCH "<contributor>.*</contributor>" ext "${var}")
string(REPLACE "<contributor>" "" contributors ${ext})
string(REPLACE "</contributor>" "" contributors ${contributors})
add_definitions(-DLocal_EACSF_CONTRIBUTORS="${contributors}")

add_subdirectory(src)

find_program(Local_EACSF_LOCATION 
      Local_EACSF
      HINTS ${CMAKE_CURRENT_BINARY_DIR}/bin)
    if(Local_EACSF_LOCATION)
      install(PROGRAMS ${Local_EACSF_LOCATION}
        DESTINATION ${INSTALL_RUNTIME_DESTINATION}
        COMPONENT RUNTIME)
    endif()


option(CREATE_BUNDLE "Create a bundle" OFF)

# Generate a bundle
if(CREATE_BUNDLE)
  
  set(CPACK_PACKAGE_VERSION ${version})

  if(APPLE)
    set(OS_BUNDLE MACOSX_BUNDLE)
  elseif(WIN32)
    set(OS_BUNDLE WIN32)
  endif()

  set(bundle_name Local_EACSF${version})

  #--------------------------------------------------------------------------------
  SET(qtconf_dest_dir bin)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${bundle_name}")

  set(CPACK_PACKAGE_ICON "${myApp_ICON}")

  IF(APPLE)
    SET(qtconf_dest_dir ${bundle_name}.app/Contents/Resources)
    SET(APPS "\${CMAKE_INSTALL_PREFIX}/${bundle_name}.app")

    set(MACOSX_BUNDLE_BUNDLE_NAME "${bundle_name}")
    set(MACOSX_BUNDLE_INFO_STRING "Local_EACSF: Local quantification of extra axial cerebrospinal fluid.")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "Local_EACSF version - ${version}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${version}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${version}")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright 2018 University of North Carolina , Chapel Hill.")
    
    set_source_files_properties(
      PROPERTIES
      MACOSX_PACKAGE_LOCATION Resources
      )
    macro(install_qt5_plugin _qt_plugin_name _qt_plugins_var)
      get_target_property(_qt_plugin_path "${_qt_plugin_name}" LOCATION)
      if(EXISTS "${_qt_plugin_path}")
        get_filename_component(_qt_plugin_file "${_qt_plugin_path}" NAME)
        get_filename_component(_qt_plugin_type "${_qt_plugin_path}" PATH)
        get_filename_component(_qt_plugin_type "${_qt_plugin_type}" NAME)
        set(_qt_plugin_dest "${bundle_name}.app/Contents/PlugIns/${_qt_plugin_type}")
        install(FILES "${_qt_plugin_path}"
          DESTINATION "${_qt_plugin_dest}"
          COMPONENT Runtime)
        set(${_qt_plugins_var}
          "${${_qt_plugins_var}};\${CMAKE_INSTALL_PREFIX}/${_qt_plugin_dest}/${_qt_plugin_file}")
      else()
        message(FATAL_ERROR "QT plugin ${_qt_plugin_name} not found")
      endif()
    endmacro()
    
    install_qt5_plugin("Qt5::QCocoaIntegrationPlugin" QT_PLUGINS)

    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    add_executable(${bundle_name} ${OS_BUNDLE} ${${APP_NAME}_src}
    )
    target_link_libraries(${bundle_name} ${QT_LIBRARIES} ${VTK_LIBRARIES})

    #--------------------------------------------------------------------------------
    # Install the QtTest application, on Apple, the bundle is at the root of the
    # install tree, and on other platforms it'll go into the bin directory.
    INSTALL(TARGETS ${bundle_name}
      DESTINATION . COMPONENT Runtime
      RUNTIME DESTINATION bin COMPONENT Runtime
    )

  ENDIF(APPLE)

  IF(WIN32)
    SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${bundle_name}.exe")
  ENDIF(WIN32)

  

  #--------------------------------------------------------------------------------
  # install a qt.conf file
  # this inserts some cmake code into the install script to write the file
  INSTALL(CODE "
      file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]\nPlugins = PlugIns\n\")
      " COMPONENT Runtime)

  # Install the license
  INSTALL(FILES 
    ${CMAKE_SOURCE_DIR}/LICENSE
    DESTINATION "${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}"
    COMPONENT Runtime)
 
  get_target_property(Qt5_location Qt5::Widgets LOCATION)
  string(FIND ${Qt5_location} "/QtWidgets" length)
  string(SUBSTRING ${Qt5_location} 0 ${length} Qt5_location)
  #Fix the bundle
  install(CODE "
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"${QT_PLUGINS};\" \"${Qt5_location}\")
    file(GLOB qt_libs \${CMAKE_INSTALL_PREFIX}/bin/*.so*)
      foreach(qt_lib \${qt_libs})
        get_filename_component(qt_file \"\${qt_lib}\" NAME)
        file(RENAME \${qt_lib} \${CMAKE_INSTALL_PREFIX}/lib/\${qt_file})
      endforeach()
   "
    COMPONENT Runtime)
 
  # To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
  # where CPackConfig.cmake is created by including CPack
  # And then there's ways to customize this as well
  set(CPACK_BINARY_DRAGNDROP ON)

endif()


if(UNIX AND NOT APPLE)
 # # when building, don't use the install RPATH already
 # # (but later on when installing)
 # SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
 # # the RPATH to be used when installing

  string(REPLACE "." ";" VERSION_LIST ${version})
  list(GET VERSION_LIST 0 CPACK_PACKAGE_VERSION_MAJOR)
  list(GET VERSION_LIST 1 CPACK_PACKAGE_VERSION_MINOR)
  list(GET VERSION_LIST 2 CPACK_PACKAGE_VERSION_PATCH)

  SET(CMAKE_INSTALL_RPATH "../lib")
  
  get_target_property(QtWidgets_location Qt5::Widgets LOCATION)
  get_target_property(QtCore_location Qt5::Core LOCATION)
  get_target_property(QtGui_location Qt5::Gui LOCATION)
  get_target_property(QtX11Extras_location Qt5::X11Extras LOCATION)
  get_target_property(QtDBus_location Qt5::DBus LOCATION)
  
  get_target_property(QtWidgets_SOFT Qt5::Widgets IMPORTED_SONAME_RELEASE)
  get_target_property(QtCore_SOFT Qt5::Core IMPORTED_SONAME_RELEASE)
  get_target_property(QtGui_SOFT Qt5::Gui IMPORTED_SONAME_RELEASE)
  get_target_property(QtX11Extras_SOFT Qt5::X11Extras IMPORTED_SONAME_RELEASE)
  get_target_property(QtDBus_SOFT Qt5::DBus IMPORTED_SONAME_RELEASE)

  get_target_property(Qt5_location Qt5::Widgets LOCATION)  
  string(FIND ${Qt5_location} "libQt5Widgets" length)
  string(SUBSTRING ${Qt5_location} 0 ${length} Qt5_location)
  
  #install(FILES ${QtWidgets_location} ${QtOpenGL_location} ${Qt5_location}/${QtWidgets_SOFT} ${Qt5_location}/${QtOpenGL_SOFT}
  #    DESTINATION lib
  #    COMPONENT Runtime)  

  install(FILES ${QtWidgets_location} ${QtCore_location} ${QtGui_location} ${QtDBus_location} ${QtX11Extras_location}
    ${Qt5_location}/${QtWidgets_SOFT} ${Qt5_location}/${QtCore_SOFT} ${Qt5_location}/${QtGui_SOFT} ${Qt5_location}/${QtX11Extras_SOFT} ${Qt5_location}/${QtDBus_SOFT}
    #${Qt5_location}/libQt5XcbQpa.so.5.5.1 ${Qt5_location}/libQt5XcbQpa.so.5
    ${Qt5_location}/libicui18n.so.56.1 ${Qt5_location}/libicuuc.so.56.1 ${Qt5_location}/libicudata.so.56.1 ${Qt5_location}/libicui18n.so.56 ${Qt5_location}/libicuuc.so.56 ${Qt5_location}/libicudata.so.56
      DESTINATION lib
      COMPONENT Runtime)

  set(QT_PLUGINS "")
  set(qt_plugins Qt5::QXcbIntegrationPlugin)
  foreach(qt_plugin ${qt_plugins})

    get_target_property(_qt_plugin_path "${qt_plugin}" LOCATION)
    get_filename_component(_qt_plugin_file "${_qt_plugin_path}" NAME)
    get_filename_component(_qt_plugin_type "${_qt_plugin_path}" PATH)
    get_filename_component(_qt_plugin_type "${_qt_plugin_type}" NAME)

    install(PROGRAMS "${_qt_plugin_path}"
          DESTINATION "bin/${_qt_plugin_type}"
          COMPONENT RuntimePlugins)
  endforeach()

  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Local_EACSF.sh "\#!/bin/bash\nSCRIPT=$(readlink -f $(realpath -s $0))\nSCRIPTPATH=$(dirname $SCRIPT)\nexport LD_LIBRARY_PATH=$SCRIPTPATH/lib:$LD_LIBRARY_PATH\n$SCRIPTPATH/bin/Local_EACSF $@\n")

  install(CODE "
    file(
      COPY ${CMAKE_CURRENT_BINARY_DIR}/Local_EACSF.sh
      DESTINATION \${CMAKE_INSTALL_PREFIX}
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )
    " COMPONENT Runtime)

endif()

# To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
# where CPackConfig.cmake is created by including CPack
# And then there's ways to customize this as well  
include(InstallRequiredSystemLibraries)
include(CPack) 


configure_file(CMake/Local_EACSFConfig.cmake.in
  "${PROJECT_BINARY_DIR}/Local_EACSFConfig.cmake" @ONLY)

if(WIN32 AND NOT CYGWIN)
  set(INSTALL_CMAKE_DIR cmake)
else()
  set(INSTALL_CMAKE_DIR lib/cmake/Local_EACSF)
endif()

install(FILES
  "${PROJECT_BINARY_DIR}/Local_EACSFConfig.cmake"  
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)


file(GLOB Local_EACSF_HEADERS "*.h")
install(FILES ${Local_EACSF_HEADERS} 
DESTINATION include)

#option(BUILD_TESTING "Build the testing tree" ON)

#IF(BUILD_TESTING)
#  include(CTest)
#  ADD_SUBDIRECTORY(Testing)
#ENDIF(BUILD_TESTING)
