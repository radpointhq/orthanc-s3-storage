# orthanc-s3-storage

Orthanc plugin providing AWS S3 and S3-compatible storage capabilities.

# Instruction, how to build the plugin

  - use a script provided with the repository inside `scripts/docker-build-orthanc`
    folder (please note, do it even if you want to build it natively, without
    Docker!)

1. Build the plugin:
```sh
$ docker build -t orthanc-s3-plugin -f scripts/docker-build-orthanc/Dockerfile .
```

# Deploying Orthanc with the plugin

  - copy `libOrthancS3StoragePlugin.dylib*` or `libOrthancS3StoragePlugin.so*` to the folder indicated in Orthanc's json config file. See official Orthanc's documentation to see how to enable plugins.

## Configuration

Place this section in the Orthanc config:

```
  "S3" : {
      "aws_access_key_id" : "XXX",
      "aws_secret_access_key" : "XXX",
      "aws_region": "name-of-region",
      "s3_bucket": "name-of-bucket",
      "s3_endpoint": "custom-s3-url",
      "implementation": "direct"
  },
```

Note that `aws_access_key_id` and `aws_secret_access_key` are optional if you provide correct creds in `~/.aws/credentials`.

You can also specify a custom endpoint so that the AWS S3 client can interface with S3-compatible services like [MinIO](https://min.io/). To run a working example you can run `make run` from the project's root directory, which will:

- Build the plugin via Docker,
- Install the plugin inside the official Orthanc image,
- Run a minIO server and an Orthanc server and configure it to use minIO as storage service.

Another optional parameter chooses between AWS Trarnsfer Manager (default) and Direct Object operations on S3.

Possible values:
- `direct`
- `transfer_manager`

## How to deploy the plugin in Docker

1. If you want to build the plugin inside a Docker container, use scripts located in `scripts/docker-build-orthanc` directory: firstly `docker-build-image.sh` and secondly `docker-run-image.sh`. See `README.md` in this directory and `Dockerfile` for more details.

2. After successfull build, the plugin will be placed inside `install` or `install-$datetime` directory and you will be able to use it, if a Linux distribution used for building shares libraries versions with a distribution, on with Orthanc is run (e.g. there is a conflict of libcrypto between Ubuntu 16.04 and Ubuntu 18.10). Therefore we advise you to build and run the plugin on the same Linux version.

3. You can use the plugin with official Orthanc's Docker image (`osimis/orthanc`), please follow instruction from [official documentation](https://osimis.atlassian.net/wiki/spaces/OKB/pages/26738689) (Section: "Example: using the osimis/orthanc images the old way"). You can also use scripts from directory `scripts/docker-run-orthanc`, see its `README.md` file.

# Additional information

1. If you want to build AWS-SDK from scratch, follow the rules, but please
note, we recommend using our configuration script and build AWS automatically.
  - `git clone https://github.com/aws/aws-sdk-cpp.git`
  - `mkdir aws-sdk-cpp-build && cd aws-sdk-cpp-build && cmake -DBUILD_ONLY="transfer;s3" ../aws-sdk-cpp  && cmake --build .  -- -j4  && make install`
  - 1GB of RAM is not enough. Adding 4GB swap file helps.

2. You might build Orthanc as well, but this step is useful only if you need
to debug the plugin
  - follow instructions from the official repo:
    https://bitbucket.org/sjodogne/orthanc/src/default/LinuxCompilation.txt
  - informations given in above file in section "Ubuntu 14.04 LTS" are fine
    for Ubuntu 16.04, too
  - you can use a script provided with the plugin from `scripts/docker-build-orthanc`
    folder, but remember to fulfill recquired dependencies

# Licensing

Copyright (C) 2018 (Radpoint Sp. z.o.o, Poland)
The S3 Storage plugin for Orthanc is licensed under the AGPL license.
