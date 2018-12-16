#!/usr/bin/env bash  
set -e

ROOT_DIR=$(git rev-parse --show-toplevel) 

pushd . >/dev/null

cd  "$ROOT_DIR" 

sudo docker build -t orthanc-s3-plugin-dev ./scripts/docker-build/

popd >/dev/null
