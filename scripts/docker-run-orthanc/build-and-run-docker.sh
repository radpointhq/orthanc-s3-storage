sudo docker build -t orthanc-s3 .
sudo docker run --rm \
-v /home/osboxes/orthanc/orthanc.json:/etc/orthanc/orthanc.json:ro \
-v /home/osboxes/orthanc/log/:/var/log/orthanc/ \
-v /home/osboxes/orthanc/orthanc-db/:/var/lib/orthanc/db/ \
-v /home/osboxes/orthanc/lib:/usr/local/lib  \
-v /home/osboxes/orthanc/plugins:/usr/local/share/orthanc/plugins \
-v /home/osboxes/orthanc/scripts:/etc/orthanc/scripts \
-e LD_LIBRARY_PATH=/usr/local/lib \
-p 4242:4242 \
-p 8042:8042 \
orthanc-s3


