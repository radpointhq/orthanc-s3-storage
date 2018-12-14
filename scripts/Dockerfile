FROM ubuntu:xenial

RUN apt-get update && \
    apt-get install -qy apt-utils && \
    apt-get install -qy  \
    build-essential git mercurial subversion cmake \
    coreutils xz-utils zip unzip curl wget \
    libcurl4-openssl-dev libssl-dev  zlib1g-dev \
    libcurl3-gnutls libgnutls30 libcurl3 

WORKDIR /app

CMD ["bash", "scripts/build-s3-docker.sh"]
