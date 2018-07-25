#ifndef UTILS_H
#define UTILS_H

#include "OrthancPluginCppWrapper.h"

#include <fstream>
#include <string>
#include <vector>

namespace OrthancPlugins {

extern OrthancPluginContext* context;

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
