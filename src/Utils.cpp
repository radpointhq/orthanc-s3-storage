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

#include "Utils.hpp"
#include "Core/OrthancException.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>

namespace OrthancPlugins {
namespace Utils {

void readFile(void** content,
              int64_t *size,
              const std::string& path) {

    if (!isRegularFile(path))
    {
        std::string s("The path does not point to a regular file: ");
        s += path;
        OrthancPluginLogError(context, s.c_str());
        throw Orthanc::OrthancException(Orthanc::ErrorCode_RegularFileExpected);
    }

    boost::filesystem::ifstream f;
    f.open(path, std::ifstream::in | std::ifstream::binary);
    if (!f.good())
    {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InexistentFile);
    }

    *size = GetStreamSize(f);
    if (*size != 0)
    {
        //it so happen that memory free'd is by
        //context->Free which in turn is ::free
        *content = malloc(*size);
        if (*content != nullptr) {
            f.read(static_cast<char*>(*content), *size);
        }
    }

    f.close();
}

void writeFile(const void* content,
               int64_t size,
               const std::string& path) {

    boost::filesystem::path boostpath (path);

    if (boost::filesystem::exists(boostpath))
    {
      // Extremely unlikely case: This Uuid has already been created
      // in the past.
      //TODO handle this case
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }

    if (boost::filesystem::exists(boostpath.parent_path()))
    {
      if (!boost::filesystem::is_directory(boostpath.parent_path()))
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_DirectoryOverFile);
      }
    }
    else
    {
      if (!boost::filesystem::create_directories(boostpath.parent_path()))
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_FileStorageCannotWrite);
      }
    }


    boost::filesystem::ofstream f;
    f.open(path, std::ofstream::out | std::ofstream::binary);
    if (!f.good())
    {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_CannotWriteFile);
    }

    if (size != 0)
    {
        f.write(reinterpret_cast<const char*>(content), size);

        if (!f.good())
        {
            f.close();
            throw Orthanc::OrthancException(Orthanc::ErrorCode_FileStorageCannotWrite);
        }
    }

    f.close();
}

void writeFile(const std::string& content,
               const std::string& path) {
    writeFile(!content.empty() ? content.c_str() : nullptr, content.size(), path);
}

void removeFile(const std::string& path) {
    if (boost::filesystem::exists(path))
    {
        if (isRegularFile(path))
        {
            boost::filesystem::remove(path);
        }
        else
        {
            throw Orthanc::OrthancException(Orthanc::ErrorCode_RegularFileExpected);
        }

        // Remove the two parent directories, ignoring the error code if
        // these directories are not empty
        try {
            boost::filesystem::remove(boost::filesystem::path(path).parent_path());
            boost::filesystem::remove(boost::filesystem::path(path).parent_path().parent_path());
        }
        catch (...)
        {
          // Ignore the error
        }
    }
}

uint64_t getFileSize(const std::string& path) {
    try
    {
        return static_cast<uint64_t>(boost::filesystem::file_size(path));
    }
    catch (boost::filesystem::filesystem_error&)
    {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InexistentFile);
    }
}

void makeDirectory(const std::string& path) {
    if (boost::filesystem::exists(path))
    {
        if (!boost::filesystem::is_directory(path))
        {
            throw Orthanc::OrthancException(Orthanc::ErrorCode_DirectoryOverFile);
        }
    }
    else
    {
        if (!boost::filesystem::create_directories(path))
        {
            throw Orthanc::OrthancException(Orthanc::ErrorCode_MakeDirectory);
        }
    }
}

int64_t GetStreamSize(std::istream& f) {
    // http://www.cplusplus.com/reference/iostream/istream/tellg/
    f.seekg(0, std::ios::end);
    int64_t size = f.tellg();
    f.seekg(0, std::ios::beg);

    return size;
}

bool isRegularFile(const std::string& path) {
    namespace fs = boost::filesystem;

    try
    {
        if (fs::exists(path))
        {
            fs::file_status status = fs::status(path);
            return (status.type() == boost::filesystem::regular_file ||
                    status.type() == boost::filesystem::reparse_file);   // Fix BitBucket issue #11
        }
    }
    catch (fs::filesystem_error&)
    {
    }

    return false;
}

bool isExistingFile(const std::string& path) {
    return boost::filesystem::exists(path);
}

bool isDirectory(const std::string& path) {
    return boost::filesystem::is_directory(path);

}

}
}
