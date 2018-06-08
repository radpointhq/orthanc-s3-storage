#!/bin/bash -xe

BUILD_DIR="orthanc-webviewer/build"
mkdir ${BUILD_DIR} || true
INSTALL_DIR="install"
mkdir ${BUILD_DIR}/${INSTALL_DIR} || true

pushd ${BUILD_DIR}

cmake .. \
 -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
 -DCMAKE_BUILD_TYPE=Debug \
 -DSTATIC_BUILD=ON \
 -DUSE_SYSTEM_BOOST="ON" \
 -DUSE_SYSTEM_ORTHANC_SDK=ON \
 -DSQLITE_SOURCE_DIR="/usr/local/opt/sqlite3" \
 -DSQLITE_INCLUDE_DIR="/usr/local/opt/sqlite3/include" \
 -DORTHANC_FRAMEWORK_SOURCE=path \
 -DORTHANC_FRAMEWORK_ROOT="${PWD}/../../orthanc"

