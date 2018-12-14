#!/usr/bin/env bash 
set -e

ROOT_DIR=$(git rev-parse --show-toplevel) 

pushd . >/dev/null

cd  "$ROOT_DIR" 

sudo docker run -v "$ROOT_DIR":/app  orthanc-s3-plugin-dev

popd  >/dev/null
