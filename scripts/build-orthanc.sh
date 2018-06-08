#!/bin/bash -xe

BUILD_DIR="orthanc/build"
mkdir ${BUILD_DIR} || true
INSTALL_DIR="install"
mkdir ${BUILD_DIR}/${INSTALL_DIR} || true

pushd ${BUILD_DIR}

cmake .. \
 -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
 -DCMAKE_BUILD_TYPE=Debug \
 -DOPENSSL_ROOT_DIR="/usr/local/opt/openssl" \
 -DSQLITE_SOURCE_DIR="/usr/local/opt/sqlite3" \
 -DSQLITE_INCLUDE_DIR="/usr/local/opt/sqlite3/include" \
 -DSTANDALONE_BUILD="ON" \
 -DSTATIC_BUILD="ON" \
 -DALLOW_DOWNLOADS="ON" \
 -DUSE_SYSTEM_BOOST="ON" \
 -DUSE_SYSTEM_MONGOOSE="OFF" \
 -DUSE_SYSTEM_PUGIXML="OFF" \
 -DUSE_SYSTEM_GOOGLE_TEST="OFF" \
 -DUSE_SYSTEM_UUID="OFF" \
 -DUSE_SYSTEM_DCMTK="OFF" \
