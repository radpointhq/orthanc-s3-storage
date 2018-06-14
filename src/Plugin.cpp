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


bool UploadFileToS3(const std::string & path);
bool DownloadFileFromS3(const std::string & path, const std::string & downloadedFileName);
bool DeleteFileAtS3(const std::string & path);


static OrthancPluginContext* context_ = NULL;
static std::string storageDir_ = "";
static std::string indexDir_ = "";

static Aws::String bucket_name_ = "bp.bucket.s3.1";
static Aws::String region_  = "eu-central-1";



static std::string GetPath(const char* uuid)
{
  return std::string(uuid);
}

static std::string GetPathStorage(const char* uuid)
{
  return storageDir_ + "/" + std::string(uuid);
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
  switch (type) {
  case     OrthancPluginContentType::OrthancPluginContentType_Dicom:
      path   = GetPathStorage(uuid);
      break;
  case OrthancPluginContentType_DicomAsJson:
      path   = GetPathInstance(uuid);
      break;
  default:
      std::cerr << "Never should happen.\n";
      path   = GetPath(uuid);
  }


  FILE* fp = fopen(path.c_str(), "wb");
  if (!fp)
  {
    return OrthancPluginErrorCode_StorageAreaPlugin;
  }

  bool ok = fwrite(content, size, 1, fp) == 1;
  fclose(fp);

  //TODO make this correct way
  //NOTE credentials are set in ~/.aws/credentials file
  if (type == OrthancPluginContentType_Dicom) {
      bool ok_s3 = UploadFileToS3(path);
  }

  return ok ? OrthancPluginErrorCode_Success : OrthancPluginErrorCode_StorageAreaPlugin;
}


static OrthancPluginErrorCode StorageRead(void** content,
                                          int64_t* size,
                                          const char* uuid,
                                          OrthancPluginContentType type)
{
    std::string path = "";
    switch (type) {
    case     OrthancPluginContentType::OrthancPluginContentType_Dicom:
        path   = GetPathStorage(uuid);
        break;
    case OrthancPluginContentType_DicomAsJson:
        path   = GetPathInstance(uuid);
        break;
    default:
        std::cerr << "Never should happen.\n";
        path   = GetPath(uuid);
    }


  FILE* fp = fopen(path.c_str(), "rb");
  if (!fp)
  {
    return OrthancPluginErrorCode_StorageAreaPlugin;
  }

  if (fseek(fp, 0, SEEK_END) < 0)
  {
    fclose(fp);
    return OrthancPluginErrorCode_StorageAreaPlugin;
  }

  *size = ftell(fp);

  if (fseek(fp, 0, SEEK_SET) < 0)
  {
    fclose(fp);
    return OrthancPluginErrorCode_StorageAreaPlugin;
  }

  bool ok = true;

  if (*size == 0)
  {
    *content = NULL;
  }
  else
  {
    *content = malloc(*size);
    if (*content == NULL ||
        fread(*content, *size, 1, fp) != 1)
    {
      ok = false;
    }
  }

  fclose(fp);

  //TODO make this correct way
  //NOTE credentials are set in ~/.aws/credentials file
  if (type == OrthancPluginContentType_Dicom) {
      std::string tempFileName = std::string("/tmp/")+path;
      bool ok_s3 = DownloadFileFromS3(path, tempFileName);
  }


  return ok ? OrthancPluginErrorCode_Success : OrthancPluginErrorCode_StorageAreaPlugin;
}


static OrthancPluginErrorCode StorageRemove(const char* uuid,
                                            OrthancPluginContentType type)
{
    std::string path = "";
    switch (type) {
    case     OrthancPluginContentType::OrthancPluginContentType_Dicom:
        path   = GetPathStorage(uuid);
        break;
    case OrthancPluginContentType_DicomAsJson:
        path   = GetPathInstance(uuid);
        break;
    default:
        std::cerr << "Never should happen.\n";
        path   = GetPath(uuid);
    }


  //TODO make this correct way
  //NOTE credentials are set in ~/.aws/credentials file
  if (type == OrthancPluginContentType_Dicom) {
      bool ok_s3 = DeleteFileAtS3(path);
  }


  if (remove(path.c_str()) == 0)
  {
    return OrthancPluginErrorCode_Success;
  }
  else
  {
    return OrthancPluginErrorCode_StorageAreaPlugin;
  }

}



bool UploadFileToS3(const std::string & path){

Aws::SDKOptions options;
Aws::InitAPI(options);
{
    const Aws::String bucket_name = bucket_name_;
    const Aws::String region = region_;
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

    // Binary files must also have the std::ios_base::bin flag or'ed in
    auto input_data = Aws::MakeShared<Aws::FStream>("PutObjectInputStream",
        file_name.c_str(), std::ios_base::in | std::ios_base::binary);

    object_request.SetBody(input_data);

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

bool DownloadFileFromS3(const std::string & path, const std::string & downloadedFileName){
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        const Aws::String bucket_name = bucket_name_;
        const Aws::String region = region_;
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

        if (get_object_outcome.IsSuccess())
        {
            Aws::OFStream local_file;
            local_file.open(downloadedFileName.c_str(), std::ios::out | std::ios::binary);
            local_file << get_object_outcome.GetResult().GetBody().rdbuf();
            std::cout << "Done!" << std::endl;
        }
        else
        {
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

    const Aws::String bucket_name = bucket_name_;
    const Aws::String region = region_;
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
    storageDir_ = configuration.GetStringValue("StorageDirectory", "");
    indexDir_ = configuration.GetStringValue("IndexDirectory", storageDir_.c_str());  //last parameter = default value

    if (storageDir_.empty()) {
        OrthancPluginLogWarning(context_, "StorageDir and IndexDir were not set.");
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

