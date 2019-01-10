
# orthanc-s3-storage

The scripts allows to run the plugin inside official Orthanc's Docker image (`osimis/orthanc`). The information are taken from [official documentation](https://osimis.atlassian.net/wiki/spaces/OKB/pages/26738689) (Section: "Example: using the osimis/orthanc images the old way"). 


In the subdirectory there is a basic structure of files and directories that is necessary to start Orthanc.  Before building Docker image, ensure that you do the following:

- in file `orthanc/orthanc.json` update a section `{ "S3": {...} }`
- copy the created plugin to `orthanc/plugins/libOrthancS3StoragePlugin.so`
- copy to the folder `orthanc/lib`  all `libaws-*` libraries from `$PLUGINDIR/install/share/orthanc/aws/lib` as well as `libcurl.so*` from `$PLUGINDIR/local/local/lib`
- with current version on aws, there should be the following libraries (plus symlinks) in `orthanc/lib` directory:
    - libaws-c-common.so
    - libaws-c-event-stream.so
    - libaws-checksums.so
    - libaws-cpp-sdk-core.so
    - libaws-cpp-sdk-s3.so
    - libaws-cpp-sdk-transfer.so
    - libcurl.so


## Running Orthanc with the plugin
Run `build-orthanc-in-docker.sh` followed by `run-orthanc-in-docker.sh` from this directory.

# Licensing
Copyright (C) 2018 (Radpoint Sp. z.o.o, Poland)
The S3 Storage plugin for Orthanc is licensed under the AGPL license. 

