#!/usr/bin/env bash  
set -e

ROOT_DIR=$(git rev-parse --show-toplevel) 

pushd . >/dev/null

cd  "$ROOT_DIR" 

docker build --no-cache -t orthanc-s3-plugin-dev ./scripts/docker-build/

popd >/dev/null
