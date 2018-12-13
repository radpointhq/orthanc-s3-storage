#!/usr/bin/env bash


die() {
echo "$*" >&2
exit 1  
}

ROOT_DIR=$(git rev-parse --show-toplevel)

if ! [ $? -eq 0 ] ; then
    echo "Set working directory inside git repo before running current script."
    exit 1
fi

pushd $(pwd)  >/dev/null
cd  "$ROOT_DIR"

CURRENT_DATETIME=$(date +"%Y-%m-%d_%H-%M-%S")
BUILD_DIR="$ROOT_DIR"/build-${CURRENT_DATETIME}
INSTALL_DIR="$ROOT_DIR"/install-${CURRENT_DATETIME}   

mkdir $BUILD_DIR || die "Error. Selected dir already exists."

cd $BUILD_DIR   || die "Cannot change dir."

cmake "$ROOT_DIR" \
 -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
 -DCMAKE_BUILD_TYPE=Debug \
 -DCMAKE_CXX_STANDARD=14 \
 -DCMAKE_CXX_STANDARD_REQUIRED="ON" \
 -DALLOW_DOWNLOADS="ON" \
 -DSTANDALONE_BUILD="ON" \
 -DSTATIC_BUILD="OFF" \
 -DUSE_SYSTEM_BOOST="OFF" \
 -DUSE_SYSTEM_CURL="ON" \
 -DUSE_SYSTEM_SSL="ON" \
 -DUSE_SYSTEM_LIBZ="ON" \
 -DUSE_SYSTEM_MONGOOSE="OFF" \
 -DUSE_SYSTEM_PUGIXML="OFF" \
 -DUSE_SYSTEM_GOOGLE_TEST="OFF" \
 -DUSE_SYSTEM_UUID="OFF" \
 -DUSE_SYSTEM_DCMTK="OFF" \
 -DUSE_SYSTEM_JSONCPP="OFF" \
 -DUSE_SYSTEM_ORTHANC_SDK="OFF" \
 -DUSE_SYSTEM_AWS_SDK="OFF"   || die "cmake error."

make -j4  || die "make error."

make install || die "make install error." 

#TODO 
# CMAKE_BUILD_TYPE - to ma byc parametr
# -DSTATIC_BUILD="OFF"  - to jest w CMakeLists
# -DALLOW_DOWNLOADS="ON" \ - w CMakeLists jest odwrotnie
# -DUSE_SYSTEM_ORTHANC_SDK="OFF" - w CmakeLists jest odwrotnie

#CMake Warning:
#  Manually-specified variables were not used by the project:
#
#    USE_SYSTEM_LIBZ
#    USE_SYSTEM_SSL

popd  >/dev/null
