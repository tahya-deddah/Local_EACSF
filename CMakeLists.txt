Project(Local_EACSF)
cmake_minimum_required(VERSION 2.8.11)

include(${CMAKE_CURRENT_SOURCE_DIR}/Common.cmake)

option( Local_EACSF_SuperBuild
        "Build Local_EACSF as a Superbuild project" ON )
if( Local_EACSF_SuperBuild )
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
else()
  include("${CMAKE_CURRENT_SOURCE_DIR}/Local_EACSF.cmake")
endif()
