/**
 * Orthanc - A Lightweight, RESTful DICOM Store
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017 Osimis, Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

#include "OrthancPluginCppWrapper.h"
#include "Timer.h"
#include "Utils.h"

#include <curl/curl.h>

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/GetBucketLocationRequest.h>

#include <aws/core/utils/logging/DefaultLogSystem.h>
#include <aws/core/utils/logging/AWSLogging.h>
#include <aws/core/utils/FileSystemUtils.h>

#include <boost/interprocess/streams/bufferstream.hpp>

#include <json/value.h>
#include <json/reader.h>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>


namespace OrthancPlugins {

bool UploadFileToS3(const std::string & path, const void* content, const int64_t& size);
bool DownloadFileFromS3(const std::string & path, void** content, int64_t* size);
bool DeleteFileFromS3(const std::string & path);

#define ALLOCATION_TAG "Orthanc S3 Storage"

OrthancPluginContext* context = nullptr;
std::string indexDir = "";

std::string s3_bucket_name = "orthanc-s3-storage";
std::string s3_region  = "eu-central-1";

std::shared_ptr<Aws::S3::S3Client> s3_client;
Aws::SDKOptions aws_api_options;
std::mutex mutex;  // protects g_i

static std::string GetPath(const char* uuid)
{
    return std::string(uuid);
}

static std::string GetPathStorage(const char* uuid)
{
  return uuid;
}

static std::string GetPathInstance(const char* uuid)
{
    return indexDir + "/" + std::string(uuid);
}


static OrthancPluginErrorCode StorageCreate(const char* uuid,
                                            const void* content,
                                            int64_t size,
                                            OrthancPluginContentType type)
{
    stopwatch<> timer;
    bool ok = false;
    std::string path;

    {
        std::stringstream ss;
        ss << "[S3] PUT: " << uuid;
        OrthancPluginLogInfo(context, ss.str().c_str());
    }

    try {
        if (type == OrthancPluginContentType_Dicom) {
            path = GetPathStorage(uuid);
            ok = UploadFileToS3(path, content, size)==EXIT_SUCCESS;
        } else if (type== OrthancPluginContentType_DicomAsJson) {
            path = GetPathInstance(uuid);
            Utils::writeFile(content, size, path);
            ok = true;
        } else {
            ok = false;
            OrthancPluginLogError(context, "[S3] Never should happen");
        }
    } catch (Orthanc::OrthancException &e) {
        std::stringstream err;
        err << "[S3] Could not open uuid: " << path << ", " << e.What();
        OrthancPluginLogError(context, err.str().c_str());
        ok = false;
    }

    {
        auto executionDuration = timer.elapsed();
        std::stringstream ss;
        ss << "[S3] PUT " << uuid << " finished in " << executionDuration << "us";
        OrthancPluginLogInfo(context, ss.str().c_str());
    }

    return ok ? OrthancPluginErrorCode_Success : OrthancPluginErrorCode_StorageAreaPlugin;
}


static OrthancPluginErrorCode StorageRead(void** content,
                                          int64_t* size,
                                          const char* uuid,
                                          OrthancPluginContentType type)
{
    stopwatch<> timer;
    bool ok = false;
    std::string path;

    {
        std::stringstream ss;
        ss << "[S3] GET: " << uuid;
        OrthancPluginLogInfo(context, ss.str().c_str());
    }

    try {
        if (type == OrthancPluginContentType_Dicom) {
            path = GetPathStorage(uuid);
            ok = DownloadFileFromS3(path, content, size)==EXIT_SUCCESS;
        } else if (type == OrthancPluginContentType_DicomAsJson) {
            path = GetPathInstance(uuid);
            Utils::readFile(content, size, path);
            ok = true;
        } else {
            OrthancPluginLogError(context, "[S3] Never should happen");
            ok = false;
        }
    } catch (Orthanc::OrthancException &e) {
        std::stringstream err;
        err << "[S3] Could not read file: " << path << ", " << e.What();
        OrthancPluginLogError(context, err.str().c_str());
        ok = false;
    }


    {
        auto executionDuration = timer.elapsed();
        std::stringstream ss;
        ss << "[S3] GET " << uuid << " finished in " << executionDuration << "us";
        OrthancPluginLogInfo(context, ss.str().c_str());
    }

    return ok ? OrthancPluginErrorCode_Success : OrthancPluginErrorCode_StorageAreaPlugin;
}


static OrthancPluginErrorCode StorageRemove(const char* uuid,
                                            OrthancPluginContentType type)
{
    bool ok = false;
    std::string path;
    stopwatch<> timer;

    {
        std::stringstream ss;
        ss << "[S3] DELETE: " << uuid;
        OrthancPluginLogInfo(context, ss.str().c_str());
    }

    try {
        if (type == OrthancPluginContentType_Dicom) {
            path = GetPathStorage(uuid);
            ok = DeleteFileFromS3(path)==EXIT_SUCCESS;
        } else if (type == OrthancPluginContentType_DicomAsJson) {
            path = GetPathInstance(uuid);
            Utils::removeFile(path);
            ok = true;
        } else {
            OrthancPluginLogError(context, "[S3] Never should happen");
            ok = false;
        }

    } catch (Orthanc::OrthancException &e) {
        std::stringstream err;
        err <<"[S3] Could not remove file: " << path << ", " << e.What();
        OrthancPluginLogError(context, err.str().c_str());
        ok = false;
    }

    {
        auto executionDuration = timer.elapsed();
        std::stringstream ss;
        ss << "[S3] DELETE; " << uuid << " finished in " << executionDuration << "us";
        OrthancPluginLogInfo(context, ss.str().c_str());
    }

    return ok ? OrthancPluginErrorCode_Success: OrthancPluginErrorCode_StorageAreaPlugin;
}

bool UploadFileToS3(const std::string & path, const void* content, const int64_t& size) {
    std::lock_guard<std::mutex> lock(mutex);


    {
        const Aws::String bucket_name = s3_bucket_name.c_str();
        const Aws::String key_name = path.c_str();
        const Aws::String file_name = path.c_str();

        Aws::S3::Model::PutObjectRequest object_request;
        object_request.WithBucket(bucket_name).WithKey(key_name);

        std::shared_ptr<Aws::IOStream> body =  std::shared_ptr<Aws::IOStream>(new boost::interprocess::bufferstream((char*)content, size));

        auto input_data = Aws::MakeShared<Aws::FStream>("PutObjectInputStream",
                                                        file_name.c_str(), std::ios_base::in | std::ios_base::binary);

        object_request.SetBody(body);

        auto put_object_outcome = s3_client->PutObject(object_request);

        if (!put_object_outcome.IsSuccess()) {
            std::stringstream err;
            err << "[S3] PUT error: " <<
                put_object_outcome.GetError().GetExceptionName() << " " <<
                put_object_outcome.GetError().GetMessage();
            OrthancPluginLogError(context, err.str().c_str());

            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

bool DownloadFileFromS3(const std::string & path, void** content, int64_t* size){
    std::lock_guard<std::mutex> lock(mutex);

    const Aws::String bucket_name = s3_bucket_name.c_str();
    const Aws::String key_name = path.c_str();

    Aws::S3::Model::GetObjectRequest object_request;
    object_request.WithBucket(bucket_name).WithKey(key_name);

    auto get_object_outcome = s3_client->GetObject(object_request);

    if (get_object_outcome.IsSuccess()) {
        *size = get_object_outcome.GetResult().GetContentLength();
        Aws::OStringStream buf;
        buf << get_object_outcome.GetResult().GetBody().rdbuf();

        //malloc because it's freed by ::free()
        *content = malloc(*size);
        if (*content!=nullptr) {
            memcpy(*content, buf.str().c_str(), *size);
        } else {
            return EXIT_FAILURE;
        }
    } else {
        std::stringstream err;
        err << "[S3] GET error: " <<
               get_object_outcome.GetError().GetExceptionName() << " " <<
               get_object_outcome.GetError().GetMessage();
        OrthancPluginLogError(context, err.str().c_str());

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

bool DeleteFileFromS3(const std::string & path) {
    std::lock_guard<std::mutex> lock(mutex);

    const Aws::String bucket_name = s3_bucket_name.c_str();
    const Aws::String key_name = path.c_str();

    Aws::S3::Model::DeleteObjectRequest object_request;
    object_request.WithBucket(bucket_name).WithKey(key_name);

    auto delete_object_outcome = s3_client->DeleteObject(object_request);

    if (!delete_object_outcome.IsSuccess()) {
        std::stringstream err;
        err << "[S3] DELETE error: " <<
            delete_object_outcome.GetError().GetExceptionName() << " " <<
            delete_object_outcome.GetError().GetMessage();
        OrthancPluginLogError(context, err.str().c_str());

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

bool readS3Configuration(OrthancPluginContext* context, std::string& s3_access_key, std::string& s3_secret_key) {
    OrthancPlugins::OrthancConfiguration configuration(context);

    //idex storage
    indexDir = configuration.GetStringValue("IndexDirectory", indexDir.c_str());  //last parameter = default value

    if (indexDir.empty()) {
        OrthancPluginLogWarning(context, "StorageDir and IndexDir were not set.");
    }

    //make sure indexDir exist
    Utils::makeDirectory(indexDir);

    // obtain S3 creds
    OrthancPlugins::OrthancConfiguration s3_configuration(context);
    if (configuration.IsSection("S3")) {
        configuration.GetSection(s3_configuration, "S3");
    } else {
        OrthancPluginLogError(context, "Can't find `S3` section in the config.");
        return false;
    }

    //required
    if (!s3_configuration.LookupStringValue(s3_access_key, "aws_access_key_id")) {
        OrthancPluginLogError(context, "AWS credentials were not set.");
        return false;
    }

    //required
    if (!s3_configuration.LookupStringValue(s3_secret_key, "aws_secret_access_key")) {
        OrthancPluginLogError(context, "AWS credentials were not set.");
        return false;
    }

    std::string region;
    if (s3_configuration.LookupStringValue(region, "aws_region")) {
        s3_region = region.c_str();
    }

    std::string bucket;
    if (s3_configuration.LookupStringValue(bucket, "s3_bucket")) {
        s3_bucket_name = bucket.c_str();
    }
    return true;
}

bool easyGET() {
    bool ok = false;
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://gazeta.pl");
        /* example.com is redirected, so we tell libcurl to follow redirection */
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK) {
            std::stringstream ss;
            ss << "curl_easy_perform() failed: %s\n" << curl_easy_strerror(res);
            OrthancPluginLogError(context, ss.str().c_str());
            ok = false;
        } else {
            ok = true;
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    return ok;
}

bool configureAwsSdk(const std::string& s3_access_key, const std::string& s3_secret_key) {

    //Enable AWS logging
    Aws::Utils::Logging::InitializeAWSLogging(
        Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>( //TODO: change log system from file to std out
            ALLOCATION_TAG, Aws::Utils::Logging::LogLevel::Info, "aws_sdk_"));

    Aws::InitAPI(aws_api_options);

    //aws_client_config.region = Aws::su
    Aws::Client::ClientConfiguration aws_client_config;
    aws_client_config.region = s3_region;
    aws_client_config.scheme = Aws::Http::Scheme::HTTPS;
    aws_client_config.connectTimeoutMs = 30000;
    aws_client_config.requestTimeoutMs = 600000;

    /*s3_client = Aws::MakeShared<Aws::S3::S3Client>(
        ALLOCATION_TAG, Aws::Auth::AWSCredentials(Aws::String(s3_access_key), Aws::String(s3_secret_key)), aws_client_config);
        */
    s3_client = Aws::MakeShared<Aws::S3::S3Client>(
        ALLOCATION_TAG, aws_client_config);

    std::stringstream ss;
    ss <<  "[S3] Checking bucket: " << s3_bucket_name;
    OrthancPluginLogInfo(context, ss.str().c_str());

    //Create bucket if it doesn't exist
    //and verify if it exists
    Aws::S3::Model::CreateBucketRequest request;
    request.SetBucket(Aws::String(s3_bucket_name));
    Aws::S3::Model::CreateBucketConfiguration req_config;
    req_config.SetLocationConstraint(Aws::S3::Model::BucketLocationConstraintMapper::GetBucketLocationConstraintForName(Aws::String(s3_region)));
    request.SetCreateBucketConfiguration(req_config);

    auto outcome = s3_client->CreateBucket(request);

    if (outcome.GetError().GetErrorType() == Aws::S3::S3Errors::BUCKET_ALREADY_OWNED_BY_YOU ||
        outcome.GetError().GetErrorType() == Aws::S3::S3Errors::BUCKET_ALREADY_EXISTS ) {
        std::stringstream ss;
        ss << "[S3] Bucket exists: " << s3_bucket_name;
        OrthancPluginLogInfo(context, ss.str().c_str());
    } else if (outcome.IsSuccess()) {
        std::stringstream ss;
        ss << "[S3] Bucket created: " << s3_bucket_name;
        OrthancPluginLogInfo(context, ss.str().c_str());
    } else {
        std::stringstream err;
        err << "[S3] Create Bucket error: " <<
            outcome.GetError().GetExceptionName() << " " <<
            outcome.GetError().GetMessage();
        OrthancPluginLogError(context, err.str().c_str());
        return false;
    }
    return true;
}

extern "C" {

ORTHANC_PLUGINS_API int32_t OrthancPluginInitialize(OrthancPluginContext* c)
{
    context = c;
    OrthancPluginLogWarning(context, "[S3] Storage plugin is initializing");

    /* Check the version of the Orthanc core */
    if (OrthancPluginCheckVersion(c) == 0)
    {
        char info[1024];
        sprintf(info, "Your version of Orthanc (%s) must be above %d.%d.%d to run this plugin",
                c->orthancVersion,
                ORTHANC_PLUGINS_MINIMAL_MAJOR_NUMBER,
                ORTHANC_PLUGINS_MINIMAL_MINOR_NUMBER,
                ORTHANC_PLUGINS_MINIMAL_REVISION_NUMBER);
        OrthancPluginLogError(context, info);
        return -1;
    }

    OrthancPluginSetDescription(context, "Implementation of S3 Storage.");

    std::string s3_access_key;
    std::string s3_secret_key;
    if (!readS3Configuration(context, s3_access_key, s3_secret_key)) {
        return EXIT_FAILURE;
    }

    /*if (!easyGET()) {
        return EXIT_FAILURE;
    }*/

    //Initialization of AWS SDK
    if (!configureAwsSdk(s3_access_key, s3_secret_key)) {
        return EXIT_FAILURE;
    }

    OrthancPluginRegisterStorageArea(context, StorageCreate, StorageRead, StorageRemove);

    return 0;
}


ORTHANC_PLUGINS_API void OrthancPluginFinalize()
{
    //Cleanup AWS logging
    Aws::Utils::Logging::ShutdownAWSLogging();
    Aws::ShutdownAPI(aws_api_options);

    OrthancPluginLogWarning(context, "[S3] Storage plugin is finalizing");
}


ORTHANC_PLUGINS_API const char* OrthancPluginGetName()
{
    return "s3-storage";
}


ORTHANC_PLUGINS_API const char* OrthancPluginGetVersion()
{
    return "1.0.0";
}

} //extern C

} //namespace
