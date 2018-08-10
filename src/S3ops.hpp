/**
 * S3 Storage Plugin - A plugin for Orthanc DICOM Server for storing
 * DICOM data in Amazon Simple Storage Service (AWS S3).
 *
 * Copyright (C) 2018 (Radpoint Sp. z o.o., Poland)
 * Marek Kwasecki, Bartłomiej Pyciński
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 **/

#ifndef S3OPS_HPP
#define S3OPS_HPP

#include "OrthancPluginCppWrapper.h"

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/core/utils/logging/AWSLogging.h>
#include <aws/transfer/TransferManager.h>

#include <algorithm>
#include <string>

namespace OrthancPlugins {

enum class S3Method {
    DIRECT,
    TRANSFER_MANAGER
};

class S3Impl {

protected:
    OrthancPluginContext* _context;
    Aws::String _bucket_name;
    //Aws::String s3_region;
    Aws::SDKOptions aws_api_options;
    std::shared_ptr<Aws::S3::S3Client> s3_client;

public:
    S3Impl(OrthancPluginContext *c): _context(c) {};
    virtual ~S3Impl() {
        //Cleanup AWS logging
        Aws::Utils::Logging::ShutdownAWSLogging();
        Aws::ShutdownAPI(aws_api_options);

        _context = nullptr;
    };

    virtual bool ConfigureAwsSdk(const std::string& s3_access_key, const std::string& s3_secret_key,
                         const std::string& _bucket_name, const std::string& s3_region);

    virtual bool UploadFileToS3(const std::string & path, const void* content, const int64_t& size) = 0;
    virtual bool DownloadFileFromS3(const std::string & path, void** content, int64_t* size) = 0;
    virtual bool DeleteFileFromS3(const std::string & path) = 0;

};

class S3Direct : public S3Impl
{
public:
    S3Direct(OrthancPluginContext *c):
        S3Impl(c) {
        LogInfo(_context, "[S3] S3Direct");
    };

    bool UploadFileToS3(const std::string & path, const void *content, const int64_t &size);
    bool DownloadFileFromS3(const std::string & path, void** content, int64_t* size);
    bool DeleteFileFromS3(const std::string & path);
};

class S3TransferManager : public S3Impl
{
    std::shared_ptr<Aws::Utils::Threading::Executor> _executor;
    std::shared_ptr<Aws::Transfer::TransferManager> _tm;

    void LogDetails(const std::shared_ptr<const Aws::Transfer::TransferHandle> &h);

public:
    S3TransferManager(OrthancPluginContext *c):
        S3Impl(c) {
        LogInfo(_context, "[S3] S3TransferManager");
    };

    bool ConfigureAwsSdk(const std::string& s3_access_key, const std::string& s3_secret_key,
                         const std::string& _bucket_name, const std::string& s3_region);

    bool UploadFileToS3(const std::string & path, const void *content, const int64_t& size);
    bool DownloadFileFromS3(const std::string & path, void** content, int64_t* size);
    bool DeleteFileFromS3(const std::string & path);
};


class S3Facade
{
    std::unique_ptr<S3Impl> _s3;

public:
    S3Facade(S3Method m, OrthancPluginContext *context);

    bool ConfigureAwsSdk(const std::string& s3_access_key, const std::string& s3_secret_key,
                         const std::string& s3_bucket_name, const std::string& s3_region) {
        return _s3->ConfigureAwsSdk(s3_access_key, s3_secret_key, s3_bucket_name, s3_region);
    };

    bool UploadFileToS3(const std::string & path, const void* content, const int64_t& size) {
        return _s3->UploadFileToS3(path, content, size);
    };
    bool DownloadFileFromS3(const std::string & path, void** content, int64_t* size) {
        return _s3->DownloadFileFromS3(path, content, size);
    };
    bool DeleteFileFromS3(const std::string & path) {
        return _s3->DeleteFileFromS3(path);
    };
};

}
#endif // S3OPS_HPP
