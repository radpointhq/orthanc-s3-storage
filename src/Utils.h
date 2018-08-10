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

#ifndef UTILS_H
#define UTILS_H

#include <fstream>
#include <string>
#include <vector>

namespace OrthancPlugins {
  namespace Utils {
    void readFile(void **content,
                  int64_t* size,
                  const std::string& path);

    void writeFile(const void* content,
                   int64_t size,
                   const std::string& path);

    void writeFile(const std::string& content,
                   const std::string& path);

    void removeFile(const std::string& path);

    uint64_t getFileSize(const std::string& path);

    void makeDirectory(const std::string& path);

    int64_t GetStreamSize(std::istream& f);
    bool isRegularFile(const std::string& path);
    bool isExistingFile(const std::string& path);
    bool isDirectory(const std::string& path);


  }

}

#endif // UTILS_HPP
