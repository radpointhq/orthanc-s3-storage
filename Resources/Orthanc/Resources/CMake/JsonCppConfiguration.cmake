set(JSONCPP_CXX11 OFF)

if (STATIC_BUILD OR NOT USE_SYSTEM_JSONCPP)
  if (USE_LEGACY_JSONCPP)
    set(JSONCPP_SOURCES_DIR ${CMAKE_BINARY_DIR}/jsoncpp-0.10.6)
    set(JSONCPP_URL "http://www.orthanc-server.com/downloads/third-party/jsoncpp-0.10.6.tar.gz")
    set(JSONCPP_MD5 "13d1991d79697df8cadbc25c93e37c83")
    add_definitions(-DORTHANC_LEGACY_JSONCPP=1)
  else()
    set(JSONCPP_SOURCES_DIR ${CMAKE_BINARY_DIR}/jsoncpp-1.8.4)
    set(JSONCPP_URL "http://www.orthanc-server.com/downloads/third-party/jsoncpp-1.8.4.tar.gz")
    set(JSONCPP_MD5 "fa47a3ab6b381869b6a5f20811198662")
    add_definitions(-DORTHANC_LEGACY_JSONCPP=0)
    set(JSONCPP_CXX11 ON)
  endif()

  DownloadPackage(${JSONCPP_MD5} ${JSONCPP_URL} "${JSONCPP_SOURCES_DIR}")

  set(JSONCPP_SOURCES
    ${JSONCPP_SOURCES_DIR}/src/lib_json/json_reader.cpp
    ${JSONCPP_SOURCES_DIR}/src/lib_json/json_value.cpp
    ${JSONCPP_SOURCES_DIR}/src/lib_json/json_writer.cpp
    )

  include_directories(
    ${JSONCPP_SOURCES_DIR}/include
    )

  source_group(ThirdParty\\JsonCpp REGULAR_EXPRESSION ${JSONCPP_SOURCES_DIR}/.*)

else()
  find_path(JSONCPP_INCLUDE_DIR json/reader.h
    /usr/include/jsoncpp
    /usr/local/include/jsoncpp
    )

  message("JsonCpp include dir: ${JSONCPP_INCLUDE_DIR}")
  include_directories(${JSONCPP_INCLUDE_DIR})
  link_libraries(jsoncpp)

  CHECK_INCLUDE_FILE_CXX(${JSONCPP_INCLUDE_DIR}/json/reader.h HAVE_JSONCPP_H)
  if (NOT HAVE_JSONCPP_H)
    message(FATAL_ERROR "Please install the libjsoncpp-dev package")
  endif()

  # Switch to the C++11 standard if the version of JsonCpp is 1.y.z
  if (EXISTS ${JSONCPP_INCLUDE_DIR}/json/version.h)
    file(STRINGS
      "${JSONCPP_INCLUDE_DIR}/json/version.h" 
      JSONCPP_VERSION_MAJOR1 REGEX
      ".*define JSONCPP_VERSION_MAJOR.*")

    if (NOT JSONCPP_VERSION_MAJOR1)
      message(FATAL_ERROR "Unable to extract the major version of JsonCpp")
    endif()
    
    string(REGEX REPLACE
      ".*JSONCPP_VERSION_MAJOR.*([0-9]+)$" "\\1" 
      JSONCPP_VERSION_MAJOR ${JSONCPP_VERSION_MAJOR1})
    message("JsonCpp major version: ${JSONCPP_VERSION_MAJOR}")

    if (JSONCPP_VERSION_MAJOR GREATER 0)
      set(JSONCPP_CXX11 ON)
    endif()
  else()
    message("Unable to detect the major version of JsonCpp, assuming < 1.0.0")
  endif()
endif()


if (JSONCPP_CXX11)
  # Osimis has encountered problems when this macro is left at its
  # default value (1000), so we increase this limit
  # https://gitlab.kitware.com/third-party/jsoncpp/commit/56df2068470241f9043b676bfae415ed62a0c172
  add_definitions(-DJSONCPP_DEPRECATED_STACK_LIMIT=5000)

  if (CMAKE_COMPILER_IS_GNUCXX OR
      "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message("Switching to C++11 standard in gcc/clang, as version of JsonCpp is >= 1.0.0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-deprecated-declarations")
  endif()
endif()
