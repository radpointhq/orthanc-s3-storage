/**
 * Orthanc - A Lightweight, RESTful DICOM Store
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2018 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * In addition, as a special exception, the copyright holders of this
 * program give permission to link the code of its release with the
 * OpenSSL project's "OpenSSL" library (or with modified versions of it
 * that use the same license as the "OpenSSL" library), and distribute
 * the linked executables. You must obey the GNU General Public License
 * in all respects for all of the code used other than "OpenSSL". If you
 * modify file(s) with this exception, you may extend this exception to
 * your version of the file(s), but you are not obligated to do so. If
 * you do not wish to do so, delete this exception statement from your
 * version. If you delete this exception statement from all source files
 * in the program, then also delete it here.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/


#pragma once

#include "Enumerations.h"

#include <stdint.h>
#include <vector>
#include <string>
#include <json/json.h>


#if !defined(ORTHANC_ENABLE_BASE64)
#  error The macro ORTHANC_ENABLE_BASE64 must be defined
#endif

#if !defined(ORTHANC_ENABLE_LOCALE)
#  error The macro ORTHANC_ENABLE_LOCALE must be defined
#endif

#if !defined(ORTHANC_ENABLE_MD5)
#  error The macro ORTHANC_ENABLE_MD5 must be defined
#endif

#if !defined(ORTHANC_ENABLE_PUGIXML)
#  error The macro ORTHANC_ENABLE_PUGIXML must be defined
#endif


/**
 * NOTE: GUID vs. UUID
 * The simple answer is: no difference, they are the same thing. Treat
 * them as a 16 byte (128 bits) value that is used as a unique
 * value. In Microsoft-speak they are called GUIDs, but call them
 * UUIDs when not using Microsoft-speak.
 * http://stackoverflow.com/questions/246930/is-there-any-difference-between-a-guid-and-a-uuid
 **/



namespace Orthanc
{
  typedef std::vector<std::string> UriComponents;

  class NullType
  {
  };

  namespace Toolbox
  {
    class LinesIterator
    {
    private:
      const std::string& content_;
      size_t             lineStart_;
      size_t             lineEnd_;

      void FindEndOfLine();
  
    public:
      LinesIterator(const std::string& content);
  
      bool GetLine(std::string& target) const;

      void Next();
    };
    
    
    void ToUpperCase(std::string& s);  // Inplace version

    void ToLowerCase(std::string& s);  // Inplace version

    void ToUpperCase(std::string& result,
                     const std::string& source);

    void ToLowerCase(std::string& result,
                     const std::string& source);

    void SplitUriComponents(UriComponents& components,
                            const std::string& uri);
  
    void TruncateUri(UriComponents& target,
                     const UriComponents& source,
                     size_t fromLevel);
  
    bool IsChildUri(const UriComponents& baseUri,
                    const UriComponents& testedUri);

    std::string AutodetectMimeType(const std::string& path);

    std::string FlattenUri(const UriComponents& components,
                           size_t fromLevel = 0);

#if ORTHANC_ENABLE_MD5 == 1
    void ComputeMD5(std::string& result,
                    const std::string& data);

    void ComputeMD5(std::string& result,
                    const void* data,
                    size_t size);
#endif

    void ComputeSHA1(std::string& result,
                     const std::string& data);

    void ComputeSHA1(std::string& result,
                     const void* data,
                     size_t size);

    bool IsSHA1(const char* str,
                size_t size);

    bool IsSHA1(const std::string& s);

#if ORTHANC_ENABLE_BASE64 == 1
    void DecodeBase64(std::string& result, 
                      const std::string& data);

    void EncodeBase64(std::string& result, 
                      const std::string& data);

    bool DecodeDataUriScheme(std::string& mime,
                             std::string& content,
                             const std::string& source);

    void EncodeDataUriScheme(std::string& result,
                             const std::string& mime,
                             const std::string& content);
#endif

#if ORTHANC_ENABLE_LOCALE == 1
    std::string ConvertToUtf8(const std::string& source,
                              Encoding sourceEncoding);

    std::string ConvertFromUtf8(const std::string& source,
                                Encoding targetEncoding);
#endif

    bool IsAsciiString(const void* data,
                       size_t size);

    bool IsAsciiString(const std::string& s);

    std::string ConvertToAscii(const std::string& source);

    std::string StripSpaces(const std::string& source);

    // In-place percent-decoding for URL
    void UrlDecode(std::string& s);

    Endianness DetectEndianness();

    std::string WildcardToRegularExpression(const std::string& s);

    void TokenizeString(std::vector<std::string>& result,
                        const std::string& source,
                        char separator);

#if ORTHANC_ENABLE_PUGIXML == 1
    void JsonToXml(std::string& target,
                   const Json::Value& source,
                   const std::string& rootElement = "root",
                   const std::string& arrayElement = "item");
#endif

    bool IsInteger(const std::string& str);

    void CopyJsonWithoutComments(Json::Value& target,
                                 const Json::Value& source);

    bool StartsWith(const std::string& str,
                    const std::string& prefix);

    void UriEncode(std::string& target,
                   const std::string& source);

    std::string GetJsonStringField(const ::Json::Value& json,
                                   const std::string& key,
                                   const std::string& defaultValue);

    bool GetJsonBooleanField(const ::Json::Value& json,
                             const std::string& key,
                             bool defaultValue);

    int GetJsonIntegerField(const ::Json::Value& json,
                            const std::string& key,
                            int defaultValue);

    unsigned int GetJsonUnsignedIntegerField(const ::Json::Value& json,
                                             const std::string& key,
                                             unsigned int defaultValue);

    bool IsUuid(const std::string& str);

    bool StartsWithUuid(const std::string& str);

#if ORTHANC_ENABLE_LOCALE == 1
    void InitializeGlobalLocale(const char* locale);

    void FinalizeGlobalLocale();

    std::string ToUpperCaseWithAccents(const std::string& source);
#endif

    std::string GenerateUuid();
  }
}




/**
 * The plain C, opaque data structure "OrthancLinesIterator" is a thin
 * wrapper around Orthanc::Toolbox::LinesIterator, and is only used by
 * "../Resources/WebAssembly/dcdict.cc", in order to avoid code
 * duplication
 **/

struct OrthancLinesIterator;

OrthancLinesIterator* OrthancLinesIterator_Create(const std::string& content);

bool OrthancLinesIterator_GetLine(std::string& target,
                                  const OrthancLinesIterator* iterator);

void OrthancLinesIterator_Next(OrthancLinesIterator* iterator);

void OrthancLinesIterator_Free(OrthancLinesIterator* iterator);
