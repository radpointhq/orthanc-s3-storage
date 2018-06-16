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
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>

#include <json/value.h>
#include <json/reader.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <boost/interprocess/streams/bufferstream.hpp>

bool UploadFileToS3(const std::string & path, const void* content, const int64_t& size);
bool DownloadFileFromS3(const std::string & path, void** content, int64_t* size);
bool DeleteFileAtS3(const std::string & path);


static OrthancPluginContext* context_ = NULL;
static std::string storageDir_ = "";
static std::string indexDir_ = "";

static std::string bucket_name_ = "";
static std::string region_ = "";



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
  return indexDir_ + "/" + std::string(uuid);
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
      //NOTE credentials are set in ~/.aws/credentials file
      //TODO make this correct way, get credentials form orthanc's json file
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
      std::cerr << "Never should happen.\n";
  }
  //return OrthancPluginErrorCode_Success;
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
          *content = malloc(*size);
          ok = !(*content == NULL || fread(*content, *size, 1, fp) != 1);
        }
        fclose(fp);
        break;
    default:
        std::cerr << "Never should happen.\n";
        path   = GetPath(uuid);
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
        ok =  DeleteFileAtS3(path)==EXIT_SUCCESS;
        break;
    case OrthancPluginContentType_DicomAsJson:
        path   = GetPathInstance(uuid);
        ok = (remove(path.c_str()) == 0);
        break;
    default:
        std::cerr << "Never should happen.\n";
        path   = GetPath(uuid);
    }
    return ok ? OrthancPluginErrorCode_Success: OrthancPluginErrorCode_StorageAreaPlugin;
}

bool UploadFileToS3(const std::string & path, const void* content, const int64_t& size){

Aws::SDKOptions options;
Aws::InitAPI(options);
{
    const Aws::String bucket_name = bucket_name_.c_str();
    const Aws::String region = region_.c_str();
    const Aws::String key_name = path.c_str();
    const Aws::String file_name = path.c_str();

    std::cout << "Uploading " << file_name << " to S3 bucket " <<
        bucket_name << " at key " << key_name << std::endl;

    Aws::Client::ClientConfiguration clientConfig;
    if (!region.empty())
        clientConfig.region = region;
    Aws::S3::S3Client s3_client(clientConfig);

    Aws::S3::Model::PutObjectRequest object_request;
    object_request.WithBucket(bucket_name).WithKey(key_name);

    std::shared_ptr<Aws::IOStream> body =  std::shared_ptr<Aws::IOStream>(new boost::interprocess::bufferstream((char*)content, size));

    auto input_data = Aws::MakeShared<Aws::FStream>("PutObjectInputStream",
             file_name.c_str(), std::ios_base::in | std::ios_base::binary);


    object_request.SetBody(body);

    auto put_object_outcome = s3_client.PutObject(object_request);

    if (put_object_outcome.IsSuccess())
    {
        std::cout << "Done!" << std::endl;
    }
    else
    {
        std::cout << "PutObject error: " <<
            put_object_outcome.GetError().GetExceptionName() << " " <<
            put_object_outcome.GetError().GetMessage() << std::endl;
    }
}
Aws::ShutdownAPI(options);
return EXIT_SUCCESS;
}

bool DownloadFileFromS3(const std::string & path, void** content, int64_t* size){
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        const Aws::String bucket_name = bucket_name_.c_str();
        const Aws::String region = region_.c_str();
        const Aws::String key_name = path.c_str();

        std::cout << "Downloading " << key_name << " from S3 bucket: " <<
            bucket_name << std::endl;

        Aws::Client::ClientConfiguration clientConfig;
        if (!region.empty())
            clientConfig.region = region;
        Aws::S3::S3Client s3_client(clientConfig);

        Aws::S3::Model::GetObjectRequest object_request;
        object_request.WithBucket(bucket_name).WithKey(key_name);

        auto get_object_outcome = s3_client.GetObject(object_request);

        bool ok = false;
        if (get_object_outcome.IsSuccess())
        {
            *size = get_object_outcome.GetResult().GetContentLength();
            Aws::OStringStream buf;
            buf << get_object_outcome.GetResult().GetBody().rdbuf();
            *content = malloc(*size);
            if (*content!=nullptr) {
                memcpy(*content, buf.str().c_str(), *size);
            } else {
                Aws::ShutdownAPI(options);
                return EXIT_FAILURE;
            }
            std::cout << "Done!" << std::endl;
        } else {
            std::cout << "GetObject error: " <<
                get_object_outcome.GetError().GetExceptionName() << " " <<
                get_object_outcome.GetError().GetMessage() << std::endl;
        }
    }

    Aws::ShutdownAPI(options);
    return EXIT_SUCCESS;
}

bool DeleteFileAtS3(const std::string & path) {
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {

    const Aws::String bucket_name = bucket_name_.c_str();
    const Aws::String region = region_.c_str();
    const Aws::String key_name = path.c_str();

    std::cout << "Deleting" << key_name << " from S3 bucket: " <<
                bucket_name << std::endl;

    Aws::Client::ClientConfiguration clientConfig;
    if (!region.empty())
        clientConfig.region = region;
    Aws::S3::S3Client s3_client(clientConfig);


    Aws::S3::Model::DeleteObjectRequest object_request;
    object_request.WithBucket(bucket_name).WithKey(key_name);

    auto delete_object_outcome = s3_client.DeleteObject(object_request);

    if (delete_object_outcome.IsSuccess())
    {
        std::cout << "Done!" << std::endl;
    }
    else
    {
        std::cout << "DeleteObject error: " <<
            delete_object_outcome.GetError().GetExceptionName() << " " <<
            delete_object_outcome.GetError().GetMessage() << std::endl;
    }
    }
    Aws::ShutdownAPI(options);
    return EXIT_SUCCESS;
}



extern "C"
{
  ORTHANC_PLUGINS_API int32_t OrthancPluginInitialize(OrthancPluginContext* c)
  {
    context_ = c;
    OrthancPluginLogWarning(context_, "S3 Storage plugin is initializing");

    /* Check the version of the Orthanc core */
    if (OrthancPluginCheckVersion(c) == 0)
    {
      char info[1024];
      sprintf(info, "Your version of Orthanc (%s) must be above %d.%d.%d to run this plugin",
              c->orthancVersion,
              ORTHANC_PLUGINS_MINIMAL_MAJOR_NUMBER,
              ORTHANC_PLUGINS_MINIMAL_MINOR_NUMBER,
              ORTHANC_PLUGINS_MINIMAL_REVISION_NUMBER);
      OrthancPluginLogError(context_, info);
      return -1;
    }


    OrthancPlugins::OrthancConfiguration configuration(context_);

    OrthancPlugins::OrthancConfiguration s3Storage;
    configuration.GetSection(s3Storage, "Storage_AWS_S3");


    //StorageDirectory not needed
    //storageDir_ = configuration.GetStringValue("StorageDirectory", "");
    //if (storageDir_.empty()) {
    //    OrthancPluginLogWarning(context_, "StorageDir was not set. Using Orthanc's defaults.");
    //}

    indexDir_ = configuration.GetStringValue("IndexDirectory", "");  //last parameter = default value
    if (indexDir_.empty()) {
        //TODO maybe force Orthanc to crash? (i.e. return 1)
        OrthancPluginLogWarning(context_, "IndexDirectory was not set. Using current working directory for index.");
        indexDir_ = ".";
    }

    bucket_name_ = s3Storage.GetStringValue("bucket_name", "");
    region_  = s3Storage.GetStringValue("aws_region", "");
    if (bucket_name_.empty() || region_.empty()) {
        OrthancPluginLogWarning(context_, "S3 credentials were not set. S3 plugin not enabled");
        //NOTE prevent Orthanc to start by changing return value to 1
        return 0;
    }

    OrthancPluginRegisterStorageArea(context_, StorageCreate, StorageRead, StorageRemove);
    return 0;
  }





  ORTHANC_PLUGINS_API void OrthancPluginFinalize()
  {
    OrthancPluginLogWarning(context_, "S3 Storage plugin is finalizing");
  }


  ORTHANC_PLUGINS_API const char* OrthancPluginGetName()
  {
    return "s3-storage";
  }


  ORTHANC_PLUGINS_API const char* OrthancPluginGetVersion()
  {
    return "1.0";
  }
}

