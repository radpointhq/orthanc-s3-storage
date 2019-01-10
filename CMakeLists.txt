cmake_minimum_required(VERSION 3.5)

option(test "Build all tests." OFF) # Makes boolean 'test' available.

project(OrthancS3Storage)

set(VERSION_MAJOR "0" )
set(VERSION_MINOR "2" )
set(VERSION_PATCH "1" )
set(PLUGIN_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

set(ORTHANC_SDK_VERSION "1.4.0")

if (PLUGIN_VERSION STREQUAL "mainline")
    set(ORTHANC_FRAMEWORK_VERSION "mainline")
    set(ORTHANC_FRAMEWORK_DEFAULT_SOURCE "path")
else()
    set(ORTHANC_FRAMEWORK_VERSION "${ORTHANC_SDK_VERSION}")
    set(ORTHANC_FRAMEWORK_DEFAULT_SOURCE "web")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
#Json Reader class is deprecated in Orthanc code
#Who cares about unused functions :P and default: in switch...
set(CMAKE_CXX_FLAGS "-Wno-switch-default -Wno-unused-function -Wno-deprecated-declarations")

if (APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++")
endif (APPLE)

set(JSONCPP_CXX11 ON) #TODO: does this work?

# Parameters of the build
set(STATIC_BUILD OFF CACHE BOOL "Static build of the third-party libraries (necessary for Windows)")
set(ALLOW_DOWNLOADS OFF CACHE BOOL "Allow CMake to download packages")
set(ORTHANC_FRAMEWORK_SOURCE "${ORTHANC_FRAMEWORK_DEFAULT_SOURCE}" CACHE STRING "Source of the Orthanc source code (can be \"hg\", \"archive\", \"web\" or \"path\")")
set(ORTHANC_FRAMEWORK_ARCHIVE "" CACHE STRING "Path to the Orthanc archive, if ORTHANC_FRAMEWORK_SOURCE is \"archive\"")
set(ORTHANC_FRAMEWORK_ROOT "" CACHE STRING "Path to the Orthanc source directory, if ORTHANC_FRAMEWORK_SOURCE is \"path\"")

# Advanced parameters to fine-tune linking against system libraries
set(USE_SYSTEM_ORTHANC_SDK ON CACHE BOOL "Use the system version of the Orthanc plugin SDK")

# Download and setup the Orthanc framework
include(${CMAKE_SOURCE_DIR}/Resources/Orthanc/DownloadOrthancFramework.cmake)

set(ORTHANC_FRAMEWORK_PLUGIN ON)
include(${ORTHANC_ROOT}/Resources/CMake/OrthancFrameworkParameters.cmake)

set(ENABLE_LOCALE ON)         # Enable support for locales (notably in Boost)
set(ENABLE_GOOGLE_TEST OFF)
set(ENABLE_PUGIXML OFF)
set(USE_BOOST_ICONV ON)
set(ENABLE_SSL ON)
set(ENABLE_WEB_CLIENT ON)

include(${ORTHANC_ROOT}/Resources/CMake/OrthancFrameworkConfiguration.cmake)
include_directories(${ORTHANC_ROOT})

include(${ORTHANC_ROOT}/Resources/CMake/DownloadPackage.cmake)
include(${ORTHANC_ROOT}/Resources/CMake/BoostConfiguration.cmake)
include(${ORTHANC_ROOT}/Resources/CMake/JsonCppConfiguration.cmake)
include(${ORTHANC_ROOT}/Resources/CMake/LibCurlConfiguration.cmake)
include(${ORTHANC_ROOT}/Resources/CMake/OpenSslConfiguration.cmake)
include(${ORTHANC_ROOT}/Resources/CMake/ZlibConfiguration.cmake)
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
    message("Including ${ORTHANC_ROOT}/Sdk-${ORTHANC_SDK_VERSION}")
    include_directories("${ORTHANC_ROOT}/Sdk-${ORTHANC_SDK_VERSION}")
else ()
    message("Checking for orthanc/OrthancCPlugin.h")
    CHECK_INCLUDE_FILE_CXX("orthanc/OrthancCPlugin.h" HAVE_ORTHANC_H)
    message("HAVE_ORTHANC_H: ${HAVE_ORTHANC_H}")
    #TODO: this check doesn't work
    #if (NOT HAVE_ORTHANC_H)
    #  message(FATAL_ERROR "Please install the headers of the Orthanc plugins SDK")
    #endif()
endif()

#create version.h
configure_file(
        "${CMAKE_SOURCE_DIR}/src/Version.hpp.in"
        "${CMAKE_BINARY_DIR}/gen/Version.hpp")
include_directories("${CMAKE_BINARY_DIR}/gen")

set(SOURCES
        src/Plugin.cpp
        src/Utils.cpp
        src/S3ops.cpp
        )

include_directories(${ORTHANC_ROOT}/Core)  # To access "OrthancException.h"
add_definitions(
        -DHAS_ORTHANC_EXCEPTION=1
        -DORTHANC_ENABLE_LOGGING_PLUGIN=1
)

#make the installed rpath able to reach aws shared libs
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "$ORIGIN/../aws/lib")

#workaround of sorts for missing symbol we're using and they don't
if (NOT USE_SYSTEM_BOOST)
    list(APPEND ORTHANC_CORE_SOURCES
            ${BOOST_NAME}/libs/filesystem/src/unique_path.cpp
            )
endif (NOT USE_SYSTEM_BOOST)

add_library(OrthancS3StoragePlugin SHARED
        ${SOURCES}
        ${ORTHANC_ROOT}/Plugins/Samples/Common/OrthancPluginCppWrapper.cpp
        ${ORTHANC_CORE_SOURCES}
        )

if (NOT USE_SYSTEM_AWS_SDK)
    add_dependencies(OrthancS3StoragePlugin aws-cpp-sdk)
endif ()

message("Setting the version of the library to ${PLUGIN_VERSION}")
set_target_properties(OrthancS3StoragePlugin PROPERTIES
        VERSION ${PLUGIN_VERSION}
        SOVERSION ${ORTHANC_SDK_VERSION}-${PLUGIN_VERSION}
        )

target_link_libraries(OrthancS3StoragePlugin)

install(
        TARGETS OrthancS3StoragePlugin
        LIBRARY DESTINATION share/orthanc/plugins    # Destination for Linux
        #LIBRARY DESTINATION lib    # Destination for Linux
)

################################
# Testing
################################
if (test)
    if (APPLE)
        add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
        add_definitions(-D__GLIBCXX__)
    endif (APPLE)

    # Download and unpack googletest at configure time
    configure_file(${CMAKE_SOURCE_DIR}/Resources/GoogleTest/CMakeLists.txt.in googletest-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
    if(result)
        message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
    if(result)
        message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    # Prevent GoogleTest from overriding our compiler/linker options
    # when building with Visual Studio
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # This adds another subdirectory, which has 'project(gtest)'.
    add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
            ${CMAKE_BINARY_DIR}/googletest-build
            EXCLUDE_FROM_ALL)

    ##############
    # Unit Tests
    ##############
    add_executable(runUnitTests
            tests/test0.cpp
            #tests/test1.cpp
            #tests/test2.cpp
            )

    set_target_properties (runUnitTests
            PROPERTIES COMPILE_FLAGS "-Wno-unused-member-function")

    target_include_directories(runUnitTests PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            )

    # Standard linking to gtest stuff.
    target_link_libraries(runUnitTests gtest gtest_main)

    # Extra linking for the project.
    #target_link_libraries(runUnitTests project1_lib)

    # This is so you can do 'make test' to see all your tests run, instead of
    # manually running the executable runUnitTests to see those specific tests.

    # You can also omit NAME and COMMAND. The second argument could be some other
    # test executable.
    add_test(S3Storage.Example runUnitTests)
    #add_test(MemStreamBuf.StreamTestSeek3a runUnitTests)
endif()
