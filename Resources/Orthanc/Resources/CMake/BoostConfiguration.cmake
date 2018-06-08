if (STATIC_BUILD OR NOT USE_SYSTEM_BOOST)
  set(BOOST_STATIC 1)
else()
  include(FindBoost)

  set(BOOST_STATIC 0)
  #set(Boost_DEBUG 1)
  #set(Boost_USE_STATIC_LIBS ON)

  if (ENABLE_LOCALE)
    list(APPEND ORTHANC_BOOST_COMPONENTS locale)
  endif()

  list(APPEND ORTHANC_BOOST_COMPONENTS filesystem thread system date_time regex)
  find_package(Boost COMPONENTS "${ORTHANC_BOOST_COMPONENTS}")

  if (NOT Boost_FOUND)
    foreach (item ${ORTHANC_BOOST_COMPONENTS})
      string(TOUPPER ${item} tmp)

      if (Boost_${tmp}_FOUND)
        set(tmp2 "found")
      else()
        set(tmp2 "missing")
      endif()
      
      message("Boost component ${item} - ${tmp2}")
    endforeach()
    
    message(FATAL_ERROR "Unable to locate Boost on this system")
  endif()

  # Boost releases 1.44 through 1.47 supply both V2 and V3 filesystem
  # http://www.boost.org/doc/libs/1_46_1/libs/filesystem/v3/doc/index.htm
  if (${Boost_VERSION} LESS 104400)
    add_definitions(
      -DBOOST_HAS_FILESYSTEM_V3=0
      )
  else()
    add_definitions(
      -DBOOST_HAS_FILESYSTEM_V3=1
      -DBOOST_FILESYSTEM_VERSION=3
      )
  endif()

  include_directories(${Boost_INCLUDE_DIRS})
  link_libraries(${Boost_LIBRARIES})
endif()


if (BOOST_STATIC)
  ##
  ## Parameters for static compilation of Boost 
  ##
  
  set(BOOST_NAME boost_1_66_0)
  set(BOOST_BCP_SUFFIX bcpdigest-1.3.2)
  set(BOOST_MD5 "e509e66140e8f2fd4d326b0052825f52")
  set(BOOST_URL "http://www.orthanc-server.com/downloads/third-party/${BOOST_NAME}_${BOOST_BCP_SUFFIX}.tar.gz")
  set(BOOST_SOURCES_DIR ${CMAKE_BINARY_DIR}/${BOOST_NAME})

  if (IS_DIRECTORY "${BOOST_SOURCES_DIR}")
    set(FirstRun OFF)
  else()
    set(FirstRun ON)
  endif()

  DownloadPackage(${BOOST_MD5} ${BOOST_URL} "${BOOST_SOURCES_DIR}")


  ##
  ## Generic configuration of Boost
  ## 

  if (CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-isystem ${BOOST_SOURCES_DIR})
  endif()

  include_directories(
    ${BOOST_SOURCES_DIR}
    )

  add_definitions(
    # Static build of Boost
    -DBOOST_ALL_NO_LIB 
    -DBOOST_ALL_NOLIB 
    -DBOOST_DATE_TIME_NO_LIB 
    -DBOOST_THREAD_BUILD_LIB
    -DBOOST_PROGRAM_OPTIONS_NO_LIB
    -DBOOST_REGEX_NO_LIB
    -DBOOST_SYSTEM_NO_LIB
    -DBOOST_LOCALE_NO_LIB
    )

  set(BOOST_SOURCES
    ${BOOST_SOURCES_DIR}/libs/system/src/error_code.cpp
    )

  if ("${CMAKE_SYSTEM_VERSION}" STREQUAL "LinuxStandardBase")
    add_definitions(
      -DBOOST_SYSTEM_USE_STRERROR=1
      )
    
    execute_process(
      COMMAND ${PATCH_EXECUTABLE} -p0 -N -i
      ${ORTHANC_ROOT}/Resources/Patches/boost-1.66.0-linux-standard-base.patch
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      RESULT_VARIABLE Failure
      )

    if (FirstRun AND Failure)
      message(FATAL_ERROR "Error while patching a file")
    endif()
  endif()

  
  ##
  ## Configuration of boost::thread
  ##
  
  if (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR
      CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR
      CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR
      CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD" OR
      CMAKE_SYSTEM_NAME STREQUAL "OpenBSD" OR
      CMAKE_SYSTEM_NAME STREQUAL "PNaCl" OR
      CMAKE_SYSTEM_NAME STREQUAL "NaCl32" OR
      CMAKE_SYSTEM_NAME STREQUAL "NaCl64")
    list(APPEND BOOST_SOURCES
      ${BOOST_SOURCES_DIR}/libs/atomic/src/lockpool.cpp
      ${BOOST_SOURCES_DIR}/libs/thread/src/pthread/once.cpp
      ${BOOST_SOURCES_DIR}/libs/thread/src/pthread/thread.cpp
      )

    if ("${CMAKE_SYSTEM_VERSION}" STREQUAL "LinuxStandardBase" OR
        CMAKE_SYSTEM_NAME STREQUAL "PNaCl" OR
        CMAKE_SYSTEM_NAME STREQUAL "NaCl32" OR
        CMAKE_SYSTEM_NAME STREQUAL "NaCl64")
      add_definitions(-DBOOST_HAS_SCHED_YIELD=1)
    endif()

  elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND BOOST_SOURCES
      ${BOOST_SOURCES_DIR}/libs/thread/src/win32/tss_dll.cpp
      ${BOOST_SOURCES_DIR}/libs/thread/src/win32/thread.cpp
      ${BOOST_SOURCES_DIR}/libs/thread/src/win32/tss_pe.cpp
      )

  elseif (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    # No support for threads in WebAssembly

  else()
    message(FATAL_ERROR "Support your platform here")
  endif()


  ##
  ## Configuration of boost::regex
  ##
  
  aux_source_directory(${BOOST_SOURCES_DIR}/libs/regex/src BOOST_REGEX_SOURCES)

  list(APPEND BOOST_SOURCES
    ${BOOST_REGEX_SOURCES}
    )


  ##
  ## Configuration of boost::datetime
  ##
  
  list(APPEND BOOST_SOURCES
    ${BOOST_SOURCES_DIR}/libs/date_time/src/gregorian/greg_month.cpp
    )


  ##
  ## Configuration of boost::filesystem
  ## 

  if (CMAKE_SYSTEM_NAME STREQUAL "PNaCl" OR
      CMAKE_SYSTEM_NAME STREQUAL "NaCl32" OR
      CMAKE_SYSTEM_NAME STREQUAL "NaCl64")
    # boost::filesystem is not available on PNaCl
    add_definitions(
      -DBOOST_HAS_FILESYSTEM_V3=0
      -D__INTEGRITY=1
      )
  else()
    add_definitions(
      -DBOOST_HAS_FILESYSTEM_V3=1
      )
    list(APPEND BOOST_SOURCES
      ${BOOST_NAME}/libs/filesystem/src/codecvt_error_category.cpp
      ${BOOST_NAME}/libs/filesystem/src/operations.cpp
      ${BOOST_NAME}/libs/filesystem/src/path.cpp
      ${BOOST_NAME}/libs/filesystem/src/path_traits.cpp
      )

    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR
        CMAKE_SYSTEM_NAME STREQUAL "OpenBSD" OR
        CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
     list(APPEND BOOST_SOURCES
        ${BOOST_SOURCES_DIR}/libs/filesystem/src/utf8_codecvt_facet.cpp
        )

    elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
      list(APPEND BOOST_SOURCES
        ${BOOST_NAME}/libs/filesystem/src/windows_file_codecvt.cpp
        )
    endif()
  endif()


  ##
  ## Configuration of boost::locale
  ## 

  if (NOT ENABLE_LOCALE)
    message("boost::locale is disabled")
  else()
    list(APPEND BOOST_SOURCES
      ${BOOST_SOURCES_DIR}/libs/locale/src/encoding/codepage.cpp
      ${BOOST_SOURCES_DIR}/libs/locale/src/shared/generator.cpp
      ${BOOST_SOURCES_DIR}/libs/locale/src/shared/date_time.cpp
      ${BOOST_SOURCES_DIR}/libs/locale/src/shared/formatting.cpp
      ${BOOST_SOURCES_DIR}/libs/locale/src/shared/ids.cpp
      ${BOOST_SOURCES_DIR}/libs/locale/src/shared/localization_backend.cpp
      ${BOOST_SOURCES_DIR}/libs/locale/src/shared/message.cpp
      ${BOOST_SOURCES_DIR}/libs/locale/src/shared/mo_lambda.cpp
      ${BOOST_SOURCES_DIR}/libs/locale/src/util/codecvt_converter.cpp
      ${BOOST_SOURCES_DIR}/libs/locale/src/util/default_locale.cpp
      ${BOOST_SOURCES_DIR}/libs/locale/src/util/gregorian.cpp
      ${BOOST_SOURCES_DIR}/libs/locale/src/util/info.cpp
      ${BOOST_SOURCES_DIR}/libs/locale/src/util/locale_data.cpp
      )        

    if (CMAKE_SYSTEM_NAME STREQUAL "OpenBSD" OR
        CMAKE_SYSTEM_VERSION STREQUAL "LinuxStandardBase")
      list(APPEND BOOST_SOURCES
        ${BOOST_SOURCES_DIR}/libs/locale/src/std/codecvt.cpp
        ${BOOST_SOURCES_DIR}/libs/locale/src/std/collate.cpp
        ${BOOST_SOURCES_DIR}/libs/locale/src/std/converter.cpp
        ${BOOST_SOURCES_DIR}/libs/locale/src/std/numeric.cpp
        ${BOOST_SOURCES_DIR}/libs/locale/src/std/std_backend.cpp
        )

      add_definitions(
        -DBOOST_LOCALE_WITH_ICONV=1
        -DBOOST_LOCALE_NO_WINAPI_BACKEND=1
        -DBOOST_LOCALE_NO_POSIX_BACKEND=1
        )
      
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR
            CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR
            CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR
            CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD" OR
            CMAKE_SYSTEM_NAME STREQUAL "PNaCl" OR
            CMAKE_SYSTEM_NAME STREQUAL "NaCl32" OR
            CMAKE_SYSTEM_NAME STREQUAL "NaCl64" OR
            CMAKE_SYSTEM_NAME STREQUAL "Emscripten") # For WebAssembly or asm.js
      list(APPEND BOOST_SOURCES
        ${BOOST_SOURCES_DIR}/libs/locale/src/posix/codecvt.cpp
        ${BOOST_SOURCES_DIR}/libs/locale/src/posix/collate.cpp
        ${BOOST_SOURCES_DIR}/libs/locale/src/posix/converter.cpp
        ${BOOST_SOURCES_DIR}/libs/locale/src/posix/numeric.cpp
        ${BOOST_SOURCES_DIR}/libs/locale/src/posix/posix_backend.cpp
        )

      add_definitions(
        -DBOOST_LOCALE_WITH_ICONV=1
        -DBOOST_LOCALE_NO_WINAPI_BACKEND=1
        -DBOOST_LOCALE_NO_STD_BACKEND=1
        )
      
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
      list(APPEND BOOST_SOURCES
        ${BOOST_SOURCES_DIR}/libs/locale/src/win32/collate.cpp
        ${BOOST_SOURCES_DIR}/libs/locale/src/win32/converter.cpp
        ${BOOST_SOURCES_DIR}/libs/locale/src/win32/lcid.cpp
        ${BOOST_SOURCES_DIR}/libs/locale/src/win32/numeric.cpp
        ${BOOST_SOURCES_DIR}/libs/locale/src/win32/win_backend.cpp
        )

      add_definitions(
        -DBOOST_LOCALE_NO_POSIX_BACKEND=1
        -DBOOST_LOCALE_NO_STD_BACKEND=1
        )

      # Starting with release 0.8.2, Orthanc statically links against
      # libiconv, even on Windows. Indeed, the "WCONV" library of
      # Windows XP seems not to support properly several codepages
      # (notably "Latin3", "Hebrew", and "Arabic"). Set
      # "USE_BOOST_ICONV" to "OFF" to use WCONV anyway.

      if (USE_BOOST_ICONV)
        add_definitions(-DBOOST_LOCALE_WITH_ICONV=1)
      else()
        add_definitions(-DBOOST_LOCALE_WITH_WCONV=1)
      endif()

    else()
      message(FATAL_ERROR "Support your platform here")
    endif()
  endif()

  
  source_group(ThirdParty\\boost REGULAR_EXPRESSION ${BOOST_SOURCES_DIR}/.*)

endif()
