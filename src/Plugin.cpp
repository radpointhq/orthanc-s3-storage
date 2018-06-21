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

//#include <orthanc/OrthancCPlugin.h>
#include "OrthancPluginCppWrapper.h"

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>

#include <aws/core/utils/logging/DefaultLogSystem.h>
#include <aws/core/utils/logging/AWSLogging.h>

#include <json/value.h>
#include <json/reader.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <boost/interprocess/streams/bufferstream.hpp>

namespace OrthancPlugins {

bool UploadFileToS3(const std::string & path, const void* content, const int64_t& size);
bool DownloadFileFromS3(const std::string & path, void** content, int64_t* size);
bool DeleteFileFromS3(const std::string & path);

#define ALLOCATION_TAG "Orthanc S3 Storage"

static OrthancPluginContext* context = NULL;
static std::string indexDir = "";

static std::string s3_bucket_name = "orthanc-s3-storage";
static std::string s3_region  = "eu-central-1";

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

  std::string path = "";
  bool ok = false;
  FILE* fp = nullptr;
  switch (type) {
  case OrthancPluginContentType_Dicom:
      path   = GetPathStorage(uuid);
      ok = UploadFileToS3(path, content, size)==EXIT_SUCCESS;
      break;
  case OrthancPluginContentType_DicomAsJson:
      //TODO change old fashion stdio to modern c++ streams
      path   = GetPathInstance(uuid);
      fp = fopen(path.c_str(), "wb");
      if (!fp)
      {
        return OrthancPluginErrorCode_StorageAreaPlugin;
      }
      ok = fwrite(content, size, 1, fp) == 1;
      fclose(fp);
      break;
  default:
      ok = false;
      OrthancPluginLogError(context, "Never should happen");
  }
  return ok ? OrthancPluginErrorCode_Success : OrthancPluginErrorCode_StorageAreaPlugin;
}


static OrthancPluginErrorCode StorageRead(void** content,
                                          int64_t* size,
                                          const char* uuid,
                                          OrthancPluginContentType type)
{
    std::string path = "";
    bool ok = false;
    FILE* fp = nullptr;
    switch (type) {
    case     OrthancPluginContentType::OrthancPluginContentType_Dicom:
        path   = GetPathStorage(uuid);
        //NOTE credentials are set in ~/.aws/credentials file
        //TODO make this correct way, get credentials form orthanc's json file
        ok = DownloadFileFromS3(path, content, size)==EXIT_SUCCESS;
        break;
    case OrthancPluginContentType_DicomAsJson:
        //TODO change old fashion stdio to modern c++ streams
        path   = GetPathInstance(uuid);
        fp = fopen(path.c_str(), "rb");
        if (!fp) {
          return OrthancPluginErrorCode_StorageAreaPlugin;
        }
        if (fseek(fp, 0, SEEK_END) < 0) {
          fclose(fp);
          return OrthancPluginErrorCode_StorageAreaPlugin;
        }
        *size = ftell(fp);
        if (fseek(fp, 0, SEEK_SET) < 0) {
          fclose(fp);
          return OrthancPluginErrorCode_StorageAreaPlugin;
        }
        if (*size == 0) {
          *content = NULL;
        } else {
          //*content = malloc(*size);
          *content = (void*) new char[*size];
          ok = !(*content == NULL || fread(*content, *size, 1, fp) != 1);
        }
        fclose(fp);
        break;
    default:
        OrthancPluginLogError(context, "Never should happen");
        ok = false;
    }
    return ok ? OrthancPluginErrorCode_Success : OrthancPluginErrorCode_StorageAreaPlugin;
}


static OrthancPluginErrorCode StorageRemove(const char* uuid,
                                            OrthancPluginContentType type)
{
    std::string path = "";
    bool ok = false;
    switch (type) {
    case     OrthancPluginContentType::OrthancPluginContentType_Dicom:
        path   = GetPathStorage(uuid);
        ok =  DeleteFileFromS3(path)==EXIT_SUCCESS;
        break;
    case OrthancPluginContentType_DicomAsJson:
        path   = GetPathInstance(uuid);
        ok = (remove(path.c_str()) == 0);
        break;
    default:
        OrthancPluginLogError(context, "Never should happen");
        ok = false;
    }
    return ok ? OrthancPluginErrorCode_Success: OrthancPluginErrorCode_StorageAreaPlugin;
}

bool UploadFileToS3(const std::string & path, const void* content, const int64_t& size) {
    std::lock_guard<std::mutex> lock(mutex);

    std::string s("[S3] PUT: ");
    s += path;
    OrthancPluginLogInfo(context, s.c_str());

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

        if (put_object_outcome.IsSuccess())
        {
            OrthancPluginLogInfo(context, "[S3] Done!");
        }
        else
        {
            std::string s("[S3] PUT error: ");
            s += put_object_outcome.GetError().GetExceptionName() + " ";
            s += put_object_outcome.GetError().GetMessage();
            OrthancPluginLogError(context, s.c_str());

            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

bool DownloadFileFromS3(const std::string & path, void** content, int64_t* size){
    std::lock_guard<std::mutex> lock(mutex);

    std::string s("[S3] GET: ");
    s += path;
    OrthancPluginLogInfo(context, s.c_str());

    {
        const Aws::String bucket_name = s3_bucket_name.c_str();
        const Aws::String key_name = path.c_str();

        Aws::S3::Model::GetObjectRequest object_request;
        object_request.WithBucket(bucket_name).WithKey(key_name);

        auto get_object_outcome = s3_client->GetObject(object_request);

        if (get_object_outcome.IsSuccess()) {
            *size = get_object_outcome.GetResult().GetContentLength();
            Aws::OStringStream buf;
            buf << get_object_outcome.GetResult().GetBody().rdbuf();
            //*content = malloc(*size);
            *content = new char[*size];
            if (*content!=nullptr) {
                memcpy(*content, buf.str().c_str(), *size);
            } else {
                return EXIT_FAILURE;
            }
            OrthancPluginLogInfo(context, "[S3] Done!");
        } else {
            std::string s("[S3] GET error: ");
            s += get_object_outcome.GetError().GetExceptionName() + " ";
            s += get_object_outcome.GetError().GetMessage();
            OrthancPluginLogError(context, s.c_str());

            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

bool DeleteFileFromS3(const std::string & path) {
    std::lock_guard<std::mutex> lock(mutex);

    std::string s("[S3] DELETE: ");
    s += path;
    OrthancPluginLogInfo(context, s.c_str());

    {
        const Aws::String bucket_name = s3_bucket_name.c_str();
        const Aws::String key_name = path.c_str();

        Aws::S3::Model::DeleteObjectRequest object_request;
        object_request.WithBucket(bucket_name).WithKey(key_name);

        auto delete_object_outcome = s3_client->DeleteObject(object_request);

        if (delete_object_outcome.IsSuccess())
        {
            OrthancPluginLogInfo(context, "[S3] Done!");
        }
        else
        {
            std::string("[S3] DELETE error: ");
            s += delete_object_outcome.GetError().GetExceptionName() + " ";
            s += delete_object_outcome.GetError().GetMessage();
            OrthancPluginLogError(context, s.c_str());

            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
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

    OrthancPlugins::OrthancConfiguration configuration(context);
    //not needed?
    indexDir = configuration.GetStringValue("IndexDirectory", indexDir.c_str());  //last parameter = default value

    if (indexDir.empty()) {
        OrthancPluginLogWarning(context, "StorageDir and IndexDir were not set.");
    }

    // obtain S3 creds
    OrthancPlugins::OrthancConfiguration s3_configuration(context);
    if (configuration.IsSection("S3")) {
        configuration.GetSection(s3_configuration, "S3");
    } else {
        OrthancPluginLogError(context, "Can't find `S3` section in the config.");
        return EXIT_FAILURE;
    }

    //required
    std::string s3_access_key;
    if (!s3_configuration.LookupStringValue(s3_access_key, "aws_access_key_id")) {
        OrthancPluginLogError(context, "AWS credentials were not set.");
        return EXIT_FAILURE;
    }

    //required
    std::string s3_secret_key;
    if (!s3_configuration.LookupStringValue(s3_secret_key, "aws_secret_access_key")) {
        OrthancPluginLogError(context, "AWS credentials were not set.");
        return EXIT_FAILURE;
    }

    std::string region;
    if (s3_configuration.LookupStringValue(region, "aws_region")) {
        s3_region = region.c_str();
    }

    std::string bucket;
    if (s3_configuration.LookupStringValue(bucket, "s3_bucket")) {
        s3_bucket_name = bucket.c_str();
    }

    //Initialization of AWS SDK
    aws_api_options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;

    Aws::InitAPI(aws_api_options);

    //Enable AWS logging
    Aws::Utils::Logging::InitializeAWSLogging(
        Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>(
            ALLOCATION_TAG, Aws::Utils::Logging::LogLevel::Trace, "aws_"));

    //aws_client_config.region = s3_region;
    Aws::Client::ClientConfiguration aws_client_config;
    aws_client_config.region = Aws::Region::EU_CENTRAL_1;
    aws_client_config.scheme = Aws::Http::Scheme::HTTPS;
    aws_client_config.connectTimeoutMs = 30000;
    aws_client_config.requestTimeoutMs = 600000;

    s3_client = Aws::MakeShared<Aws::S3::S3Client>(
        ALLOCATION_TAG, Aws::Auth::AWSCredentials(s3_access_key, s3_secret_key), aws_client_config);

    auto outcome = s3_client->ListBuckets();
    if (outcome.IsSuccess()) {
        auto buckets = outcome.GetResult().GetBuckets();
        for (auto i : buckets) {
            std::cout << "Bucket: " << i.GetName() << '\n';
        }
        std::cout << std::endl;
    } else {
        std::string s("[S3] List Buckets error: ");
        s += outcome.GetError().GetExceptionName() + " ";
        s += outcome.GetError().GetMessage();
        OrthancPluginLogError(context, s.c_str());
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
    return "1.0";
}

} //extern C

} //namespace
