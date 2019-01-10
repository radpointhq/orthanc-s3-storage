#!/usr/bin/env bash 
set -e

ROOT_DIR=$(git rev-parse --show-toplevel) 

pushd . >/dev/null

cd  "$ROOT_DIR" 

docker run -v "$ROOT_DIR":/app -v "$ROOT_DIR"/local:/artifacts/     orthanc-s3-plugin-dev

popd  >/dev/null
