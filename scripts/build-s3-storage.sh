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

BUILD_DIR="orthanc-s3-storage/build"
mkdir ${BUILD_DIR} || true
INSTALL_DIR="install"
mkdir ${BUILD_DIR}/${INSTALL_DIR} || true

pushd ${BUILD_DIR}

# Use this combination if you have orthank branch laying around
# -DUSE_SYSTEM_ORTHANC_SDK="ON" \
# -DORTHANC_FRAMEWORK_SOURCE="path" \
# -DORTHANC_FRAMEWORK_ROOT="$PWD/../../orthanc" \

# Use this combination in case you want it to be downloaded
# -DUSE_SYSTEM_ORTHANC_SDK="OFF" \
# -DORTHANC_S3STORAGE_VERSION="1.4.0" \

cmake .. \
 -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
 -DCMAKE_BUILD_TYPE=Debug \
 -DCMAKE_CXX_STANDARD=11 \
 -DCMAKE_CXX_STANDARD_REQUIRED="ON" \
 -DALLOW_DOWNLOADS="ON" \
 -DSTANDALONE_BUILD="ON" \
 -DSTATIC_BUILD="OFF" \
 -DUSE_SYSTEM_BOOST="ON" \
 -DUSE_SYSTEM_CURL="ON" \
 -DUSE_SYSTEM_MONGOOSE="OFF" \
 -DUSE_SYSTEM_PUGIXML="OFF" \
 -DUSE_SYSTEM_GOOGLE_TEST="OFF" \
 -DUSE_SYSTEM_UUID="OFF" \
 -DUSE_SYSTEM_DCMTK="OFF" \
 -DUSE_SYSTEM_JSONCPP="OFF" \
 -DUSE_SYSTEM_AWS_SDK="OFF" \
 -DUSE_SYSTEM_ORTHANC_SDK="OFF" \
 -DORTHANC_S3STORAGE_VERSION="1.4.0" \
 #-DORTHANC_FRAMEWORK_SOURCE="path" \
 #-DORTHANC_FRAMEWORK_ROOT="$PWD/../../orthanc" \
