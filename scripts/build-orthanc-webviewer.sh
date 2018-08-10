#!/bin/bash -xe

# S3 Storage Plugin - A plugin for Orthanc DICOM Server for storing
# DICOM data in Amazon Simple Storage Service (AWS S3).
#
# Copyright (C) 2018 (Radpoint Sp. z o.o., Poland)
# Marek Kwasecki, Bartłomiej Pyciński
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

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

