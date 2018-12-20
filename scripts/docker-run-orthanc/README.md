
# orthanc-s3-storage

The scripts allows to run the plugin inside official Orthanc's Docker image (`osimis/orthanc`). The information are taken from [official documentation](https://osimis.atlassian.net/wiki/spaces/OKB/pages/26738689) (Section: "Example: using the osimis/orthanc images the old way"). 

Before building Docker image, ensure that there exist the following files or directories:

- `$HOME/orthanc/orthanc.json` - single configuration file with a section `{ "S3": {...} }`
- `$HOME/orthanc/plugins/libOrthancS3StoragePlugin.so` - the plugin
- `$HOME/orthanc/lib` - contains all `libaws` libraries copied from `$PLUGINDIR/install/share/orthanc/aws/lib`

## Running Orthanc with the plugin

Run `build-and-run-docker.sh` from this directory.

# Licensing
Copyright (C) 2018 (Radpoint Sp. z.o.o, Poland)
The S3 Storage plugin for Orthanc is licensed under the AGPL license. 


