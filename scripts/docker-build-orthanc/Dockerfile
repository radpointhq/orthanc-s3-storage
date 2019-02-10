FROM ubuntu:xenial

RUN apt-get update && \
    apt-get install -qy apt-utils && \
    apt-get install -qy  \
    build-essential git mercurial cmake \
    coreutils xz-utils zip unzip wget \
    libssl-dev  zlib1g-dev \
    libgnutls30 


#build custom version on curl that doesn't depend on libgnutls
#the version of curl is the same as the one linked by orthanc (Orthanc-1.4.0/Resources/CMake/LibCurlConfiguration.cmake)
RUN wget http://www.orthanc-server.com/downloads/third-party/curl-7.57.0.tar.gz  && \
tar xzf curl-7.57.0.tar.gz   && \
cd curl-7.57.0  && \
./configure --prefix=/usr/local --disable-ldaps --disable-ldap --without-gnutls --without-librtmp && \
make -j4 && \
make install

WORKDIR /app

CMD ["bash", "scripts/docker-build-orthanc/build-s3-plugin.sh"]
