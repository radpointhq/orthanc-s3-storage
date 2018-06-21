cmake_minimum_required(VERSION 3.0)

project(OrthancS3Storage)

set(ORTHANC_S3STORAGE_VERSION "mainline" CACHE STRING "Version of the library and Orthanc SDK")

if (ORTHANC_S3STORAGE_VERSION STREQUAL "mainline")
  set(ORTHANC_FRAMEWORK_VERSION "mainline")
  set(ORTHANC_FRAMEWORK_DEFAULT_SOURCE "path")
else()
  set(ORTHANC_FRAMEWORK_VERSION "1.3.2")
  set(ORTHANC_FRAMEWORK_DEFAULT_SOURCE "web")
endif()

set(CMAKE_CXX_STANDARD 11)
set(JSONCPP_CXX11 ON) #TODO: does this work?

# Parameters of the build
set(STATIC_BUILD OFF CACHE BOOL "Static build of the third-party libraries (necessary for Windows)")
set(ALLOW_DOWNLOADS OFF CACHE BOOL "Allow CMake to download packages")
set(ORTHANC_FRAMEWORK_SOURCE "${ORTHANC_FRAMEWORK_DEFAULT_SOURCE}" CACHE STRING "Source of the Orthanc source code (can be \"hg\", \"archive\", \"web\" or \"path\")")
set(ORTHANC_FRAMEWORK_ARCHIVE "" CACHE STRING "Path to the Orthanc archive, if ORTHANC_FRAMEWORK_SOURCE is \"archive\"")
set(ORTHANC_FRAMEWORK_ROOT "" CACHE STRING "Path to the Orthanc source directory, if ORTHANC_FRAMEWORK_SOURCE is \"path\"")

# Advanced parameters to fine-tune linking against system libraries
set(USE_SYSTEM_BOOST ON CACHE BOOL "Use the system version of Boost")
set(USE_SYSTEM_JSONCPP ON CACHE BOOL "Use the system version of JsonCpp")
set(USE_SYSTEM_ORTHANC_SDK ON CACHE BOOL "Use the system version of the Orthanc plugin SDK")

# Download and setup the Orthanc framework
include(${CMAKE_SOURCE_DIR}/Resources/Orthanc/DownloadOrthancFramework.cmake)

set(ORTHANC_FRAMEWORK_PLUGIN ON)
include(${ORTHANC_ROOT}/Resources/CMake/OrthancFrameworkParameters.cmake)

set(ENABLE_LOCALE ON)         # Enable support for locales (notably in Boost)
set(ENABLE_GOOGLE_TEST OFF)
set(ENABLE_PUGIXML OFF)
set(USE_BOOST_ICONV ON)

include(${ORTHANC_ROOT}/Resources/CMake/OrthancFrameworkConfiguration.cmake)
include_directories(${ORTHANC_ROOT})

include(${ORTHANC_ROOT}/Resources/CMake/DownloadPackage.cmake)
include(${ORTHANC_ROOT}/Resources/CMake/BoostConfiguration.cmake)
include(${ORTHANC_ROOT}/Resources/CMake/JsonCppConfiguration.cmake)
include(${ORTHANC_ROOT}/Resources/CMake/LibCurlConfiguration.cmake)
include(${ORTHANC_ROOT}/Resources/CMake/OpenSslConfiguration.cmake)
include(${CMAKE_SOURCE_DIR}/Resources/CMake/AwsSdkConfiguration.cmake)

message("ORTHANC_ROOT: ${ORTHANC_ROOT}")
message("STATIC_BUILD: ${STATIC_BUILD}")
message("USE_SYSTEM_ORTHANC_SDK: ${USE_SYSTEM_ORTHANC_SDK}")
list(LENGTH ORTHANC_CORE_SOURCES COUNT_CORE_SOURCES)
message("Elements in ORTHANC_CORE_SOURCES: ${COUNT_CORE_SOURCES}")

# Check that the Orthanc SDK headers are available
include_directories(${ORTHANC_ROOT}/Plugins/Include) #necessary for orthanc/OrthancCPLugin.h
include_directories(${ORTHANC_ROOT}/Plugins/Samples/Common) #necessary for OrthancPluginCppWrapper.h and friends

if (STATIC_BUILD OR NOT USE_SYSTEM_ORTHANC_SDK)
  message("Including ${ORTHANC_ROOT}/Sdk-1.3.2")
  include_directories(${ORTHANC_ROOT}/Sdk-1.3.2)
else ()
  message("Checking for orthanc/OrthancCPlugin.h")
  CHECK_INCLUDE_FILE_CXX("orthanc/OrthancCPlugin.h" HAVE_ORTHANC_H)
  message("HAVE_ORTHANC_H: ${HAVE_ORTHANC_H}")
  #TODO: this check doesn't work
  #if (NOT HAVE_ORTHANC_H)
  #  message(FATAL_ERROR "Please install the headers of the Orthanc plugins SDK")
  #endif()
endif()

set(SOURCES 
  src/Plugin.cpp
  )

include_directories(${ORTHANC_ROOT}/Core)  # To access "OrthancException.h"
add_definitions(
  -DHAS_ORTHANC_EXCEPTION=1
  )

add_library(OrthancS3StoragePlugin SHARED 
    ${SOURCES}
    ${ORTHANC_ROOT}/Plugins/Samples/Common/OrthancPluginCppWrapper.cpp
    ${CURL_SOURCES}
    ${OPENSSL_SOURCES}
    ${ORTHANC_CORE_SOURCES}
    )

add_dependencies(OrthancS3StoragePlugin aws-cpp-sdk-project)

message("Setting the version of the library to ${ORTHANC_S3STORAGE_VERSION}")
add_definitions(-DORTHANC_S3STORAGE_VERSION="${ORTHANC_S3STORAGE_VERSION}")
set_target_properties(OrthancS3StoragePlugin PROPERTIES 
  VERSION ${ORTHANC_S3STORAGE_VERSION} 
  SOVERSION ${ORTHANC_S3STORAGE_VERSION}
  )

target_link_libraries(OrthancS3StoragePlugin)

install(
  TARGETS OrthancS3StoragePlugin
  RUNTIME DESTINATION lib    # Destination for Windows
  LIBRARY DESTINATION share/orthanc/plugins    # Destination for Linux
  )
