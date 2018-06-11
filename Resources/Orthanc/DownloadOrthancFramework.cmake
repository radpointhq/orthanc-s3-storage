# Orthanc - A Lightweight, RESTful DICOM Store
# Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
# Department, University Hospital of Liege, Belgium
# Copyright (C) 2017-2018 Osimis S.A., Belgium
#
# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# In addition, as a special exception, the copyright holders of this
# program give permission to link the code of its release with the
# OpenSSL project's "OpenSSL" library (or with modified versions of it
# that use the same license as the "OpenSSL" library), and distribute
# the linked executables. You must obey the GNU General Public License
# in all respects for all of the code used other than "OpenSSL". If you
# modify file(s) with this exception, you may extend this exception to
# your version of the file(s), but you are not obligated to do so. If
# you do not wish to do so, delete this exception statement from your
# version. If you delete this exception statement from all source files
# in the program, then also delete it here.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.



##
## Check whether the parent script sets the mandatory variables
##

if (NOT DEFINED ORTHANC_FRAMEWORK_SOURCE OR
    (NOT ORTHANC_FRAMEWORK_SOURCE STREQUAL "hg" AND
     NOT ORTHANC_FRAMEWORK_SOURCE STREQUAL "web" AND
     NOT ORTHANC_FRAMEWORK_SOURCE STREQUAL "archive" AND
     NOT ORTHANC_FRAMEWORK_SOURCE STREQUAL "path"))
  message(FATAL_ERROR "The variable ORTHANC_FRAMEWORK_SOURCE must be set to \"hg\", \"web\", \"archive\" or \"path\"")
endif()


##
## Detection of the requested version
##

if (ORTHANC_FRAMEWORK_SOURCE STREQUAL "hg" OR
    ORTHANC_FRAMEWORK_SOURCE STREQUAL "archive" OR
    ORTHANC_FRAMEWORK_SOURCE STREQUAL "web")
  if (NOT DEFINED ORTHANC_FRAMEWORK_VERSION)
    message(FATAL_ERROR "The variable ORTHANC_FRAMEWORK_VERSION must be set")
  endif()

  if (DEFINED ORTHANC_FRAMEWORK_MAJOR OR
      DEFINED ORTHANC_FRAMEWORK_MINOR OR
      DEFINED ORTHANC_FRAMEWORK_REVISION OR
      DEFINED ORTHANC_FRAMEWORK_MD5)
    message(FATAL_ERROR "Some internal variable has been set")
  endif()

  set(ORTHANC_FRAMEWORK_MD5 "")

  if (ORTHANC_FRAMEWORK_VERSION STREQUAL "mainline")
    set(ORTHANC_FRAMEWORK_BRANCH "default")

  else()
    set(ORTHANC_FRAMEWORK_BRANCH "Orthanc-${ORTHANC_FRAMEWORK_VERSION}")

    set(RE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
    string(REGEX REPLACE ${RE} "\\1" ORTHANC_FRAMEWORK_MAJOR ${ORTHANC_FRAMEWORK_VERSION})
    string(REGEX REPLACE ${RE} "\\2" ORTHANC_FRAMEWORK_MINOR ${ORTHANC_FRAMEWORK_VERSION})
    string(REGEX REPLACE ${RE} "\\3" ORTHANC_FRAMEWORK_REVISION ${ORTHANC_FRAMEWORK_VERSION})

    if (NOT ORTHANC_FRAMEWORK_MAJOR MATCHES "^[0-9]+$" OR
        NOT ORTHANC_FRAMEWORK_MINOR MATCHES "^[0-9]+$" OR
        NOT ORTHANC_FRAMEWORK_REVISION MATCHES "^[0-9]+$")
      message("Bad version of the Orthanc framework: ${ORTHANC_FRAMEWORK_VERSION}")
    endif()

    if (ORTHANC_FRAMEWORK_VERSION STREQUAL "1.3.1")
      set(ORTHANC_FRAMEWORK_MD5 "dac95bd6cf86fb19deaf4e612961f378")
    elseif (ORTHANC_FRAMEWORK_VERSION STREQUAL "1.3.2")
      set(ORTHANC_FRAMEWORK_MD5 "d0ccdf68e855d8224331f13774992750")
    endif()
  endif()
endif()



##
## Detection of the third-party software
##

if (ORTHANC_FRAMEWORK_SOURCE STREQUAL "hg")
  find_program(ORTHANC_FRAMEWORK_HG hg)
  
  if (${ORTHANC_FRAMEWORK_HG} MATCHES "ORTHANC_FRAMEWORK_HG-NOTFOUND")
    message(FATAL_ERROR "Please install Mercurial")
  endif()
endif()


if (ORTHANC_FRAMEWORK_SOURCE STREQUAL "archive" OR
    ORTHANC_FRAMEWORK_SOURCE STREQUAL "web")
  if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
    find_program(ORTHANC_FRAMEWORK_7ZIP 7z 
      PATHS 
      "$ENV{ProgramFiles}/7-Zip"
      "$ENV{ProgramW6432}/7-Zip"
      )

    if (${ORTHANC_FRAMEWORK_7ZIP} MATCHES "ORTHANC_FRAMEWORK_7ZIP-NOTFOUND")
      message(FATAL_ERROR "Please install the '7-zip' software (http://www.7-zip.org/)")
    endif()

  else()
    find_program(ORTHANC_FRAMEWORK_TAR tar)
    if (${ORTHANC_FRAMEWORK_TAR} MATCHES "ORTHANC_FRAMEWORK_TAR-NOTFOUND")
      message(FATAL_ERROR "Please install the 'tar' package")
    endif()
  endif()
endif()



##
## Case of the Orthanc framework specified as a path on the filesystem
##

if (ORTHANC_FRAMEWORK_SOURCE STREQUAL "path")
  if (NOT DEFINED ORTHANC_FRAMEWORK_ROOT)
    message(FATAL_ERROR "The variable ORTHANC_FRAMEWORK_ROOT must provide the path to the sources of Orthanc")
  endif()
  
  if (NOT EXISTS ${ORTHANC_FRAMEWORK_ROOT})
    message(FATAL_ERROR "Non-existing directory: ${ORTHANC_FRAMEWORK_ROOT}")
  endif()
  
  if (NOT EXISTS ${ORTHANC_FRAMEWORK_ROOT}/Resources/CMake/OrthancFrameworkParameters.cmake)
    message(FATAL_ERROR "Directory not containing the source code of Orthanc: ${ORTHANC_FRAMEWORK_ROOT}")
  endif()
  
  set(ORTHANC_ROOT ${ORTHANC_FRAMEWORK_ROOT})
endif()



##
## Case of the Orthanc framework cloned using Mercurial
##

if (ORTHANC_FRAMEWORK_SOURCE STREQUAL "hg")
  if (NOT STATIC_BUILD AND NOT ALLOW_DOWNLOADS)
    message(FATAL_ERROR "CMake is not allowed to download from Internet. Please set the ALLOW_DOWNLOADS option to ON")
  endif()

  set(ORTHANC_ROOT ${CMAKE_BINARY_DIR}/orthanc)

  if (EXISTS ${ORTHANC_ROOT})
    message("Updating the Orthanc source repository using Mercurial")
    execute_process(
      COMMAND ${ORTHANC_FRAMEWORK_HG} pull
      WORKING_DIRECTORY ${ORTHANC_ROOT}
      RESULT_VARIABLE Failure
      )    
  else()
    message("Forking the Orthanc source repository using Mercurial")
    execute_process(
      COMMAND ${ORTHANC_FRAMEWORK_HG} clone "https://bitbucket.org/sjodogne/orthanc"
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      RESULT_VARIABLE Failure
      )    
  endif()

  if (Failure OR NOT EXISTS ${ORTHANC_ROOT})
    message(FATAL_ERROR "Cannot fork the Orthanc repository")
  endif()

  message("Setting branch of the Orthanc repository to: ${ORTHANC_FRAMEWORK_BRANCH}")

  execute_process(
    COMMAND ${ORTHANC_FRAMEWORK_HG} update -c ${ORTHANC_FRAMEWORK_BRANCH}
    WORKING_DIRECTORY ${ORTHANC_ROOT}
    RESULT_VARIABLE Failure
    )

  if (Failure)
    message(FATAL_ERROR "Error while running Mercurial")
  endif()
endif()



##
## Case of the Orthanc framework provided as a source archive on the
## filesystem
##

if (ORTHANC_FRAMEWORK_SOURCE STREQUAL "archive")
  if (NOT DEFINED ORTHANC_FRAMEWORK_ARCHIVE)
    message(FATAL_ERROR "The variable ORTHANC_FRAMEWORK_ARCHIVE must provide the path to the sources of Orthanc")
  endif()
endif()



##
## Case of the Orthanc framework downloaded from the Web
##

if (ORTHANC_FRAMEWORK_SOURCE STREQUAL "web")
  if (DEFINED ORTHANC_FRAMEWORK_URL)
    string(REGEX REPLACE "^.*/" "" ORTHANC_FRAMEMORK_FILENAME "${ORTHANC_FRAMEWORK_URL}")
  else()
    # Default case: Download from the official Web site
    set(ORTHANC_FRAMEMORK_FILENAME Orthanc-${ORTHANC_FRAMEWORK_VERSION}.tar.gz)
    #set(ORTHANC_FRAMEWORK_URL "http://www.orthanc-server.com/downloads/get.php?path=/orthanc/${ORTHANC_FRAMEMORK_FILENAME}")
    set(ORTHANC_FRAMEWORK_URL "http://www.orthanc-server.com/downloads/third-party/orthanc-framework/${ORTHANC_FRAMEMORK_FILENAME}")
  endif()

  set(ORTHANC_FRAMEWORK_ARCHIVE "${CMAKE_SOURCE_DIR}/ThirdPartyDownloads/${ORTHANC_FRAMEMORK_FILENAME}")

  if (NOT EXISTS "${ORTHANC_FRAMEWORK_ARCHIVE}")
    if (NOT STATIC_BUILD AND NOT ALLOW_DOWNLOADS)
      message(FATAL_ERROR "CMake is not allowed to download from Internet. Please set the ALLOW_DOWNLOADS option to ON")
    endif()

    message("Downloading: ${ORTHANC_FRAMEWORK_URL}")

    file(DOWNLOAD
      "${ORTHANC_FRAMEWORK_URL}" "${ORTHANC_FRAMEWORK_ARCHIVE}" 
      SHOW_PROGRESS EXPECTED_MD5 "${ORTHANC_FRAMEWORK_MD5}"
      TIMEOUT 60
      INACTIVITY_TIMEOUT 60
      )
  else()
    message("Using local copy of: ${ORTHANC_FRAMEWORK_URL}")
  endif()  
endif()




##
## Uncompressing the Orthanc framework, if it was retrieved from a
## source archive on the filesystem, or from the official Web site
##

if (ORTHANC_FRAMEWORK_SOURCE STREQUAL "archive" OR
    ORTHANC_FRAMEWORK_SOURCE STREQUAL "web")

  if (NOT DEFINED ORTHANC_FRAMEWORK_ARCHIVE OR
      NOT DEFINED ORTHANC_FRAMEWORK_VERSION OR
      NOT DEFINED ORTHANC_FRAMEWORK_MD5)
    message(FATAL_ERROR "Internal error")
  endif()

  if (ORTHANC_FRAMEWORK_MD5 STREQUAL "")
    message(FATAL_ERROR "Unknown release of Orthanc: ${ORTHANC_FRAMEWORK_VERSION}")
  endif()

  file(MD5 ${ORTHANC_FRAMEWORK_ARCHIVE} ActualMD5)

  if (NOT "${ActualMD5}" STREQUAL "${ORTHANC_FRAMEWORK_MD5}")
    message(FATAL_ERROR "The MD5 hash of the Orthanc archive is invalid: ${ORTHANC_FRAMEWORK_ARCHIVE}")
  endif()

  set(ORTHANC_ROOT "${CMAKE_BINARY_DIR}/Orthanc-${ORTHANC_FRAMEWORK_VERSION}")

  if (NOT IS_DIRECTORY "${ORTHANC_ROOT}")
    if (NOT ORTHANC_FRAMEWORK_ARCHIVE MATCHES ".tar.gz$")
      message(FATAL_ERROR "Archive should have the \".tar.gz\" extension: ${ORTHANC_FRAMEWORK_ARCHIVE}")
    endif()
    
    message("Uncompressing: ${ORTHANC_FRAMEWORK_ARCHIVE}")

    if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
      # How to silently extract files using 7-zip
      # http://superuser.com/questions/331148/7zip-command-line-extract-silently-quietly

      execute_process(
        COMMAND ${ORTHANC_FRAMEWORK_7ZIP} e -y ${ORTHANC_FRAMEWORK_ARCHIVE}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        RESULT_VARIABLE Failure
        OUTPUT_QUIET
        )
      
      if (Failure)
        message(FATAL_ERROR "Error while running the uncompression tool")
      endif()

      get_filename_component(TMP_FILENAME "${ORTHANC_FRAMEWORK_ARCHIVE}" NAME)
      string(REGEX REPLACE ".gz$" "" TMP_FILENAME2 "${TMP_FILENAME}")

      execute_process(
        COMMAND ${ORTHANC_FRAMEWORK_7ZIP} x -y ${TMP_FILENAME2}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        RESULT_VARIABLE Failure
        OUTPUT_QUIET
        )

    else()
      execute_process(
        COMMAND sh -c "${ORTHANC_FRAMEWORK_TAR} xfz ${ORTHANC_FRAMEWORK_ARCHIVE}"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        RESULT_VARIABLE Failure
        )
    endif()
   
    if (Failure)
      message(FATAL_ERROR "Error while running the uncompression tool")
    endif()

    if (NOT IS_DIRECTORY "${ORTHANC_ROOT}")
      message(FATAL_ERROR "The Orthanc framework was not uncompressed at the proper location. Check the CMake instructions.")
    endif()
  endif()
endif()
