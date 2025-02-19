FROM orthanc-s3-plugin as s3-plugin-builder

FROM jodogne/orthanc as orthanc

COPY --from=s3-plugin-builder /app/build/libOrthancS3StoragePlugin.so /usr/local/share/orthanc/plugins/
COPY --from=s3-plugin-builder /app/install/share/orthanc/aws/lib/libaws-cpp-sdk-*.so /app/install/share/orthanc/aws/lib/
COPY --from=s3-plugin-builder /usr/local/lib/libcurl.so.4 /usr/local/lib/
COPY --from=s3-plugin-builder /usr/lib/x86_64-linux-gnu/libssl.so.1.1 /usr/lib/x86_64-linux-gnu/libssl.so.1.1
COPY --from=s3-plugin-builder /usr/lib/x86_64-linux-gnu/libcrypto.so.1.1 /usr/lib/x86_64-linux-gnu/libcrypto.so.1.1
