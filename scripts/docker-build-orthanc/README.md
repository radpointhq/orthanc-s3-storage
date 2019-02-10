The folder contains files which allow to build the plugin inside a docker 
container. You can also build the plugin in the native envirinment, just run
only a script `build-s3-docker.sh`. First you need to comment final lines
from the file (they are Docker-related) to avoid build error.

If you want use Docker, then execute the following scripts:

    docker-build-image.sh
    docker-run-image.sh

After build process there will be created directories `build` and `install` 
containing all the files. These folders are mapped as Docker volumes, so to 
remove them, one needs root priviledges. 

If you need to, customize the build process by editing the following file 
(before building the image):

    build-s3-docker.sh

