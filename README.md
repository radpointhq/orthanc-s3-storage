# orthanc-s3-storage

Orthanc plugin providing AWS S3 storage

# Instruction, how to build the plugin

## Prerequisits
1. Build AWS-SDK from scratch (optional, you can do this by the plugin's CMakeLists system)
  - `git clone https://github.com/aws/aws-sdk-cpp.git`
  - `cmake -DCMAKE_INSTALL_PREFIX=install -DBUILD_ONLY="transfer s3" -H aws-sdk-cpp -B aws-sdk-cpp/build && cmake --build aws-sdk-cpp/build -- -j4`
  - Either `make install`, or set paths maually later
  - 1GB of RAM is not enough. Adding 4GB swap file helps.

2. Build orthanc (optional if you don't need to debug plugin)
  - follow instructions from the official repo: https://bitbucket.org/sjodogne/orthanc/src/default/LinuxCompilation.txt
  - informations given in above filne in section "Ubuntu 14.04 LTS" are fne for Ubuntu 16.04, too
  - you can use a script provided with the plugin from `scripts` folder, but remember to fulfill recquired dependencies

## Build just a plugin
  - use script provided with the plugin from `scripts` folder
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
1. The plugin was compiled on Ubuntu 16.04, with the following CMake options (the most important): 
   - `-D STATIC_BUILD=OFF`
   - `-D USE_SYSTEM_CURL=ON` (if `OFF`, strange linker errors appeared)
   - `-D USE_SYSTEM_everything-else=OFF`

2. `mkdir $HOME/orthanc`, 

3. Create and edit `$HOME/orthanc/orthanc.json` file. Set the following options:

    { /* ... */
    "StorageDirectory" : "/var/lib/orthanc/db",
    "IndexDirectory" : "/var/lib/orthanc/db",
    "Plugins" : [ "/usr/local/share/orthanc/plugins" ],
    "S3" : {}, //see above
    /* .... */
    }

3. Copy the following files from the machine on which the plugin was compiled to the machine, where Docker runs:
   - `scp libOrthancS3StoragePlugin.so*  DOCKER_HOST:orthanc/plugins/`
   - `scp aws-sdk-cpp-1.4.70-build/aws-cpp-sdk-core/libaws-cpp-sdk-core.so  aws-sdk-cpp-1.4.70-build/aws-cpp-sdk-s3/libaws-cpp-sdk-s3.so    DOCKER_HOST:orthanc/lib/`

   
4. Copy the following files (inside the host machine, where Docker runs):
   - I khow, it is a dirty hack, but it works
   - `cp /usr/lib/x86_64-linux-gnu/libstdc++.so*  $HOME/orthanc/lib/`

5. Use official `jodogne/docker` image, most options are cloned from [Orthanc manual](http://book.orthanc-server.com/users/docker.html):

6. Run Orthanc!

    docker run \
    -v /home/ubuntu/orthanc/orthanc.json:/etc/orthanc/orthanc.json:ro \
    -v /home/ubuntu/orthanc/log/:/var/log/orthanc/ \
    -v /home/ubuntu/orthanc/orthanc-db/:/var/lib/orthanc/db/ \
    -v /home/ubuntu/orthanc/plugins:/usr/local/share/orthanc/plugins \
    -v /home/ubuntu/orthanc/scripts:/etc/orthanc/scripts \
    -v /home/ubuntu/orthanc/lib:/usr/local/lib \
    -e LD_LIBRARY_PATH=/usr/local/lib \
    --name orthanc \
    -p 4242:4242 \
    -p 8042:8042   \
    --detach \
    jodogne/orthanc  --logdir=/var/log/orthanc /etc/orthanc/orthanc.json

7. Always check last log just after you start:  `grep "Orthanc has started" $HOME/orthanc/log/Orthanc.log` 


# Licensing
The S3 Storage plugin for Orthanc is licensed under the AGPL license. 
