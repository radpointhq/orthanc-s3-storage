The folder contains files which allow to build the plugin inside a docker container.

Just run the following scripts:

    docker-build-image.sh
    docker-run-image.sh

After build process there will be created directories `build-$datetime` and `install-$datetime` containing all the files. These folders are mapped as Docker volumes, so to remove them, one needs root priviledges. Note, that you can change the value of the variable `USE_EXISTING_BUILD` in the script `build-s3-docker.sh` to re-use previous build files and to avoid full rebuild.

If you need to, customize the build process by editing the following file (before building the image):

    build-s3-docker.sh

