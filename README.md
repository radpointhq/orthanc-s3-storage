# orthanc-s3-storage

Orthanc plugin providing AWS S3 storage

# Instruction, how to build the plugin

## Prerequisits (optional)
1. Build AWS-SDK from scratch (optional, as you can do this step using the plugin's CMakeLists system)
  - `git clone https://github.com/aws/aws-sdk-cpp.git`
  - `cmake -DCMAKE_INSTALL_PREFIX=install -DBUILD_ONLY="transfer s3" -H aws-sdk-cpp -B aws-sdk-cpp/build && cmake --build aws-sdk-cpp/build -- -j4`
  - Either `make install`, or set paths maually later
  - 1GB of RAM is not enough. Adding 4GB swap file helps.

2. Build orthanc (optional if you don't need to debug the plugin)
  - follow instructions from the official repo: https://bitbucket.org/sjodogne/orthanc/src/default/LinuxCompilation.txt
  - informations given in above file in section "Ubuntu 14.04 LTS" are fine for Ubuntu 16.04, too
  - you can use a script provided with the plugin from `scripts/docker-build` folder, but remember to fulfill recquired dependencies
  
## Build just the plugin
  - use script provided with the plugin from `scripts/docker-build` folder (even if you want to build it natively, without Docker!)
  - copy `libOrthancS3StoragePlugin.dylib*` or `libOrthancS3StoragePlugin.so*` to the folder indicated in Orthanc's json config file


## Configuration

Place this section in the Orthanc config:

```
  "S3" : {
      "aws_access_key_id" : "XXX",
      "aws_secret_access_key" : "XXX",
      "aws_region": "name-of-region",
      "s3_bucket": "name-of-bucker",
      "implementation": "direct"
  },
```

Note that `aws_access_key_id` and `aws_secret_access_key` are optional if you provide correct creds in `~/.aws/credentials`.

Another optional parameter chooses between AWS Trarnsfer Manager (default) and Direct Object operations on S3. 
Possible values:
 - `direct`
 - `transfer_manager`


## How to deploy the plugin in Docker

1. If you want to build the plugin inside a Docker's container, use scripts located in `scripts/docker-build` directory: firstly `docker-build-image.sh` and secondly `docker-run-image.sh`. See `README.md` in this directory and `Dockerfile` for more details.

2. After successfull build, the plugin will be placed inside `install` or `install-$datetime` directory and you will be able to use it, if a Linux distribution used for building shares libraries versions with a distributon, on with Orthanc is run (e.g. there is a conflict of libcrypto between Ubuntu 16.04 and Ubuntu 18.10). THerefore we advise building and running the plugin on the same version of Linux.

3. You can use the plugin with official Orthanc's Docker image (`osimis/orthanc`), please follow instruction from [official documentation](https://osimis.atlassian.net/wiki/spaces/OKB/pages/26738689) (Section: "Example: using the osimis/orthanc images the old way"). You can also use scripts from directory `scripts/docker-run-orthanc`, see its `README.md` file.

# Licensing
Copyright (C) 2018 (Radpoint Sp. z.o.o, Poland) 
The S3 Storage plugin for Orthanc is licensed under the AGPL license. 
