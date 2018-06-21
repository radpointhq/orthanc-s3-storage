# orthanc-s3-storage

Orthanc plugin providing AWS S3 storage

# Instruction, how to build the plugin

## Prerequisits
1. Build AWS-SDK from scratch (optional)
  - `git clone https://github.com/aws/aws-sdk-cpp.git`
  - `cmake -DCMAKE_INSTALL_PREFIX=install -DBUILD_ONLY=s3 -H aws-sdk-cpp -B aws-sdk-cpp/build && cmake --build aws-sdk-cpp/build -- -j4`
  - I didn't `make install`, so I need to set paths maually later
  - 1GB of RAM is not enough. Adding 4GB swap file helped.

2. Build orthanc
  - TODO:
  - use script provided with the plugin from scripts folder

## Build just a plugin
  - use script provided with the plugin from scripts folder
  - copy `libOrthancS3StoragePlugin.dylib*` or `libOrthancS3StoragePlugin.so*` to the folder indicated in Orthanc's json config file

## Full build of Orthanc

If you follow this way, you can build Orthanc with QtCreator. Then you can debug all the stuff.

1. update main `CMakeLists.txt` of Orthanc to include new plugin (file `CMakeLists.txt-main-level` inside `sss` directory)
2. build whole orthanc (remember flag `-j` of `make`!) with `cmake`. If you get into troubles, check [Orthanc's manual](http://book.orthanc-server.com/faq/compiling.html). 
3. the plugins appears in `Orthanc_build` directory

