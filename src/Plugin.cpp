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

#include "OrthancPluginCppWrapper.h"
#include "Version.hpp"
#include "Timer.hpp"
#include "Utils.hpp"
#include "S3ops.hpp"

#include <boost/algorithm/string.hpp>

#include <json/value.h>
#include <json/reader.h>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

#define AWS_DEFAULT_REGION "eu-central-1"
#define AWS_DEFAULT_BUCKET_MAME "delme-test-bucket"

namespace OrthancPlugins {

struct S3PluginContext {
    std::string s3_access_key;
    std::string s3_secret_key;

    std::string s3_region;
    std::string s3_bucket_name;

    S3Method s3_method = S3Method::DIRECT;
};

OrthancPluginContext* context = nullptr;

//std::unique_ptr<S3Facade> s3;
static std::unique_ptr<S3Impl> s3;
static std::string indexDir = "";

static std::string GetPathStorage(const char* uuid)
{
  return uuid;
}

static std::string GetPathInstance(const char* uuid)
{
    std::string level_1 (uuid, 0, 2);
    std::string level_2 (uuid, 2, 2);
    return indexDir + "/" + level_1 + "/" + level_2  + "/" + std::string(uuid);

}


static OrthancPluginErrorCode StorageCreate(const char* uuid,
                                            const void* content,
                                            int64_t size,
                                            OrthancPluginContentType type)
{
    Stopwatch timer;
    bool ok = false;
    std::string path;

    {
        std::stringstream ss;
        ss << "[S3] PUT: " << uuid << " begin";
        LogInfo(context, ss.str().c_str());
    }

    try {
        path = GetPathStorage(uuid);
        ok = s3->UploadFileToS3(path, content, size);
    } catch (Orthanc::OrthancException &e) {
        std::stringstream err;
        err << "[S3] Could not open uuid: " << path << ", " << e.What();
        LogError(context, err.str().c_str());
        ok = false;
    }

    {
        auto executionDuration = timer.elapsed();
        std::stringstream ss;
        ss << "[S3] PUT " << uuid << " finished in " << executionDuration << "us";
        LogInfo(context, ss.str().c_str());
    }

    return ok ? OrthancPluginErrorCode_Success : OrthancPluginErrorCode_StorageAreaPlugin;
}


static OrthancPluginErrorCode StorageRead(void** content,
                                          int64_t* size,
                                          const char* uuid,
                                          OrthancPluginContentType type)
{
    Stopwatch timer;
    bool ok = false;
    std::string path;

    {
        std::stringstream ss;
        ss << "[S3] GET: " << uuid;
        LogInfo(context, ss.str().c_str());
    }

    try {
        path = GetPathStorage(uuid);
        ok = s3->DownloadFileFromS3(path, content, size);
    } catch (Orthanc::OrthancException &e) {
        std::stringstream err;
        err << "[S3] Could not read file: " << path << ", " << e.What();
        LogError(context, err.str().c_str());
        ok = false;
    }


    {
        auto executionDuration = timer.elapsed();
        std::stringstream ss;
        ss << "[S3] GET " << uuid << " finished in " << executionDuration << "us";
        LogInfo(context, ss.str().c_str());
    }

    return ok ? OrthancPluginErrorCode_Success : OrthancPluginErrorCode_StorageAreaPlugin;
}


static OrthancPluginErrorCode StorageRemove(const char* uuid,
                                            OrthancPluginContentType type)
{
    bool ok = false;
    std::string path;
    Stopwatch timer;

    {
        std::stringstream ss;
        ss << "[S3] DELETE: " << uuid;
        LogInfo(context, ss.str().c_str());
    }

    try {
        path = GetPathStorage(uuid);
        ok = s3->DeleteFileFromS3(path);
    } catch (Orthanc::OrthancException &e) {
        std::stringstream err;
        err <<"[S3] Could not remove file: " << path << ", " << e.What();
        LogError(context, err.str().c_str());
        ok = false;
    }

    {
        auto executionDuration = timer.elapsed();
        std::stringstream ss;
        ss << "[S3] DELETE; " << uuid << " finished in " << executionDuration << "us";
        LogInfo(context, ss.str().c_str());
    }

    return ok ? OrthancPluginErrorCode_Success: OrthancPluginErrorCode_StorageAreaPlugin;
}

bool readS3Configuration(OrthancPluginContext* context, S3PluginContext& c) {

    OrthancPlugins::OrthancConfiguration configuration(context);

    //idex storage
    indexDir = configuration.GetStringValue("IndexDirectory", indexDir.c_str());  //last parameter = default value

    if (indexDir.empty()) {
        LogWarning(context, "StorageDir and IndexDir were not set.");
    }

    //make sure indexDir exist
    Utils::makeDirectory(indexDir);

    // obtain S3 creds
    OrthancPlugins::OrthancConfiguration s3_configuration(context);
    if (configuration.IsSection("S3")) {
        configuration.GetSection(s3_configuration, "S3");
    } else {
        LogError(context, "Can't find `S3` section in the config.");
        return false;
    }

    c.s3_access_key = s3_configuration.GetStringValue("aws_access_key_id", "").c_str();
    c.s3_secret_key = s3_configuration.GetStringValue("aws_secret_access_key", "").c_str();

    c.s3_region = s3_configuration.GetStringValue("aws_region", AWS_DEFAULT_REGION).c_str();
    c.s3_bucket_name = s3_configuration.GetStringValue("s3_bucket", AWS_DEFAULT_BUCKET_MAME).c_str();

    std::string method;
    s3_configuration.LookupStringValue(method, "implementation");
    if (boost::iequals(method,"direct")) {
        c.s3_method = S3Method::DIRECT;
    } else if (boost::iequals(method, "transfer_manager")) {
        c.s3_method = S3Method::TRANSFER_MANAGER;
    } // else default is DIRECT


    // Log stuff
    if (!c.s3_access_key.empty())
        LogInfo(context, "[S3] Aws Access Key set");
    if (!c.s3_secret_key.empty())
        LogInfo(context, "[S3] Aws Secret Key set");

    std::stringstream log_region;
    log_region << "[S3] Aws region: " << c.s3_region;
    LogInfo(context, log_region.str().c_str());

    std::stringstream log_bucket;
    log_bucket << "[S3] Aws bucket: " << c.s3_bucket_name;
    LogInfo(context, log_bucket.str().c_str());

    return true;
}


extern "C" {

ORTHANC_PLUGINS_API int32_t OrthancPluginInitialize(OrthancPluginContext* pluginContext)
{
    context = pluginContext;
    LogWarning(context, "[S3] Storage plugin is initializing");

    /* Check the version of the Orthanc core */
    if (OrthancPluginCheckVersion(context) == 0)
    {
        std::stringstream ss;
        ss << "Your version of Orthanc (" << context->orthancVersion << ") must be above "
        << ORTHANC_PLUGINS_MINIMAL_MAJOR_NUMBER << '.'
        << ORTHANC_PLUGINS_MINIMAL_MINOR_NUMBER << '.'
        << ORTHANC_PLUGINS_MINIMAL_REVISION_NUMBER << '.'
        << " to run this plugin";
        LogError(context, ss.str());
        return -1;
    }

    OrthancPluginSetDescription(context, "Implementation of S3 Storage.");

    S3PluginContext c;
    if (!readS3Configuration(context, c)) {
        return EXIT_FAILURE;
    }

    //Initialization of AWS SDK
    //s3 = std::unique_ptr<S3Facade>(new S3Facade(c.s3_method, context));
    if (c.s3_method == S3Method::DIRECT) {
        s3 = std::unique_ptr<S3Impl>(new S3Direct(context));
    } else {//if (c.s3_method == S3Method::TRANSFER_MANAGER) {
        s3 = std::unique_ptr<S3Impl>(new S3TransferManager(context));
    }

    if (!s3->ConfigureAwsSdk(c.s3_access_key, c.s3_secret_key, c.s3_bucket_name, c.s3_region)) {
        return EXIT_FAILURE;
    }

    OrthancPluginRegisterStorageArea(context, StorageCreate, StorageRead, StorageRemove);

    return 0;
}


ORTHANC_PLUGINS_API void OrthancPluginFinalize()
{
    s3.release();

    LogWarning(context, "[S3] Storage plugin is finalizing");
}


ORTHANC_PLUGINS_API const char* OrthancPluginGetName()
{
    return NAME;
}


ORTHANC_PLUGINS_API const char* OrthancPluginGetVersion()
{
    return PLUGIN_VERSION;
}

} //extern C

} //namespace
