The folder contains files which allow to build the plugin.

## Dependencies

Install the following dependencies before building:

```
sudo apt install -y cmake make g++ unzip libcurl libcurl4-openssl-dev zlib1g-dev
```

## Inside a docker container

If you want use Docker, then execute the following scripts:

```
./docker-build-image.sh
./docker-run-image.sh
```

After build process there will be created directories `build` and `install` 
containing all the files. These folders are mapped as Docker volumes, so to 
remove them, one needs root priviledges. 

If you need to, customize the build process by editing the following file 
(before building the image):

```
vi build-s3-plugin.sh
```

## Native 

You can also build the plugin in the native envirinment.  Modify the `build-s3-plugin.sh` file and comment out the following lines:

```
#WARN the following two lines should be run only if the build is performed from docker environment
#cp -r /usr/local /artifacts/
#chmod -R a+w  /artifacts
```

Then run the script.
