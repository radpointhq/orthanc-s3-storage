FROM osimis/orthanc

ENV HTTP_BUNDLE_DEFAULTS=false
ENV LISTENER_BUNDLE_DEFAULTS=false
ENV PLUGINS_BUNDLE_DEFAULTS=false
ENV STORAGE_BUNDLE_DEFAULTS=false
ENV AC_BUNDLE_DEFAULTS=false

ENV LD_LIBRARY_PATH=/usr/local/lib

run apt-get update && apt-get install -qy libssl-dev
