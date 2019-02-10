docker run --network=host  --rm \
-v `pwd`/orthanc/orthanc.json:/etc/orthanc/orthanc.json:ro \
-v `pwd`/orthanc/log/:/var/log/orthanc/ \
-v `pwd`/orthanc/orthanc-db/:/var/lib/orthanc/db/ \
-v `pwd`/orthanc/lib:/usr/local/lib  \
-v `pwd`/orthanc/plugins:/usr/local/share/orthanc/plugins \
-v `pwd`/orthanc/scripts:/etc/orthanc/scripts \
-e LD_LIBRARY_PATH=/usr/local/lib \
-p 4242:4242 \
-p 8042:8042 \
orthanc-s3 --verbose --trace /etc/orthanc/orthanc.json  --logdir=/var/log/orthanc
