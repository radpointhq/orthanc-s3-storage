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


#include "PrecompiledHeaders.h"
#include "Toolbox.h"

#include "OrthancException.h"
#include "Logging.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp> 
#include <boost/uuid/sha1.hpp>
 
#include <string>
#include <stdint.h>
#include <string.h>
#include <algorithm>
#include <ctype.h>


#if ORTHANC_ENABLE_MD5 == 1
#  include "../Resources/ThirdParty/md5/md5.h"
#endif

#if ORTHANC_ENABLE_BASE64 == 1
#  include "../Resources/ThirdParty/base64/base64.h"
#endif

#if ORTHANC_ENABLE_LOCALE == 1
#  include <boost/locale.hpp>
#endif


#if defined(_MSC_VER) && (_MSC_VER < 1800)
// Patch for the missing "_strtoll" symbol when compiling with Visual Studio < 2013
extern "C"
{
  int64_t _strtoi64(const char *nptr, char **endptr, int base);
  int64_t strtoll(const char *nptr, char **endptr, int base)
  {
    return _strtoi64(nptr, endptr, base);
  } 
}
#endif


#if defined(_WIN32)
#  include <windows.h>   // For ::Sleep
#endif


#if ORTHANC_ENABLE_PUGIXML == 1
#  include "ChunkedBuffer.h"
#  include <pugixml.hpp>
#endif


// Inclusions for UUID
// http://stackoverflow.com/a/1626302

extern "C"
{
#if defined(_WIN32)
#  include <rpc.h>
#else
#  include <uuid/uuid.h>
#endif
}



namespace Orthanc
{
  void Toolbox::LinesIterator::FindEndOfLine()
  {
    lineEnd_ = lineStart_;

    while (lineEnd_ < content_.size() &&
           content_[lineEnd_] != '\n' &&
           content_[lineEnd_] != '\r')
    {
      lineEnd_ += 1;
    }
  }
  

  Toolbox::LinesIterator::LinesIterator(const std::string& content) :
    content_(content),
    lineStart_(0)
  {
    FindEndOfLine();
  }

    
  bool Toolbox::LinesIterator::GetLine(std::string& target) const
  {
    assert(lineStart_ <= content_.size() &&
           lineEnd_ <= content_.size() &&
           lineStart_ <= lineEnd_);

    if (lineStart_ == content_.size())
    {
      return false;
    }
    else
    {
      target = content_.substr(lineStart_, lineEnd_ - lineStart_);
      return true;
    }
  }

    
  void Toolbox::LinesIterator::Next()
  {
    lineStart_ = lineEnd_;

    if (lineStart_ != content_.size())
    {
      assert(content_[lineStart_] == '\r' ||
             content_[lineStart_] == '\n');

      char second;
      
      if (content_[lineStart_] == '\r')
      {
        second = '\n';
      }
      else
      {
        second = '\r';
      }
        
      lineStart_ += 1;

      if (lineStart_ < content_.size() &&
          content_[lineStart_] == second)
      {
        lineStart_ += 1;
      }

      FindEndOfLine();
    }
  }

  
  void Toolbox::ToUpperCase(std::string& s)
  {
    std::transform(s.begin(), s.end(), s.begin(), toupper);
  }


  void Toolbox::ToLowerCase(std::string& s)
  {
    std::transform(s.begin(), s.end(), s.begin(), tolower);
  }


  void Toolbox::ToUpperCase(std::string& result,
                            const std::string& source)
  {
    result = source;
    ToUpperCase(result);
  }

  void Toolbox::ToLowerCase(std::string& result,
                            const std::string& source)
  {
    result = source;
    ToLowerCase(result);
  }


  void Toolbox::SplitUriComponents(UriComponents& components,
                                   const std::string& uri)
  {
    static const char URI_SEPARATOR = '/';

    components.clear();

    if (uri.size() == 0 ||
        uri[0] != URI_SEPARATOR)
    {
      throw OrthancException(ErrorCode_UriSyntax);
    }

    // Count the number of slashes in the URI to make an assumption
    // about the number of components in the URI
    unsigned int estimatedSize = 0;
    for (unsigned int i = 0; i < uri.size(); i++)
    {
      if (uri[i] == URI_SEPARATOR)
        estimatedSize++;
    }

    components.reserve(estimatedSize - 1);

    unsigned int start = 1;
    unsigned int end = 1;
    while (end < uri.size())
    {
      // This is the loop invariant
      assert(uri[start - 1] == '/' && (end >= start));

      if (uri[end] == '/')
      {
        components.push_back(std::string(&uri[start], end - start));
        end++;
        start = end;
      }
      else
      {
        end++;
      }
    }

    if (start < uri.size())
    {
      components.push_back(std::string(&uri[start], end - start));
    }

    for (size_t i = 0; i < components.size(); i++)
    {
      if (components[i].size() == 0)
      {
        // Empty component, as in: "/coucou//e"
        throw OrthancException(ErrorCode_UriSyntax);
      }
    }
  }


  void Toolbox::TruncateUri(UriComponents& target,
                            const UriComponents& source,
                            size_t fromLevel)
  {
    target.clear();

    if (source.size() > fromLevel)
    {
      target.resize(source.size() - fromLevel);

      size_t j = 0;
      for (size_t i = fromLevel; i < source.size(); i++, j++)
      {
        target[j] = source[i];
      }

      assert(j == target.size());
    }
  }
  


  bool Toolbox::IsChildUri(const UriComponents& baseUri,
                           const UriComponents& testedUri)
  {
    if (testedUri.size() < baseUri.size())
    {
      return false;
    }

    for (size_t i = 0; i < baseUri.size(); i++)
    {
      if (baseUri[i] != testedUri[i])
        return false;
    }

    return true;
  }


  std::string Toolbox::AutodetectMimeType(const std::string& path)
  {
    std::string contentType;
    size_t lastDot = path.rfind('.');
    size_t lastSlash = path.rfind('/');

    if (lastDot == std::string::npos ||
        (lastSlash != std::string::npos && lastDot < lastSlash))
    {
      // No trailing dot, unable to detect the content type
    }
    else
    {
      const char* extension = &path[lastDot + 1];
    
      // http://en.wikipedia.org/wiki/Mime_types
      // Text types
      if (!strcmp(extension, "txt"))
        contentType = "text/plain";
      else if (!strcmp(extension, "html"))
        contentType = "text/html";
      else if (!strcmp(extension, "xml"))
        contentType = "text/xml";
      else if (!strcmp(extension, "css"))
        contentType = "text/css";

      // Application types
      else if (!strcmp(extension, "js"))
        contentType = "application/javascript";
      else if (!strcmp(extension, "json"))
        contentType = "application/json";
      else if (!strcmp(extension, "pdf"))
        contentType = "application/pdf";

      // Images types
      else if (!strcmp(extension, "jpg") || !strcmp(extension, "jpeg"))
        contentType = "image/jpeg";
      else if (!strcmp(extension, "gif"))
        contentType = "image/gif";
      else if (!strcmp(extension, "png"))
        contentType = "image/png";
    }

    return contentType;
  }


  std::string Toolbox::FlattenUri(const UriComponents& components,
                                  size_t fromLevel)
  {
    if (components.size() <= fromLevel)
    {
      return "/";
    }
    else
    {
      std::string r;

      for (size_t i = fromLevel; i < components.size(); i++)
      {
        r += "/" + components[i];
      }

      return r;
    }
  }


#if ORTHANC_ENABLE_MD5 == 1
  static char GetHexadecimalCharacter(uint8_t value)
  {
    assert(value < 16);

    if (value < 10)
    {
      return value + '0';
    }
    else
    {
      return (value - 10) + 'a';
    }
  }


  void Toolbox::ComputeMD5(std::string& result,
                           const std::string& data)
  {
    if (data.size() > 0)
    {
      ComputeMD5(result, &data[0], data.size());
    }
    else
    {
      ComputeMD5(result, NULL, 0);
    }
  }


  void Toolbox::ComputeMD5(std::string& result,
                           const void* data,
                           size_t size)
  {
    md5_state_s state;
    md5_init(&state);

    if (size > 0)
    {
      md5_append(&state, 
                 reinterpret_cast<const md5_byte_t*>(data), 
                 static_cast<int>(size));
    }

    md5_byte_t actualHash[16];
    md5_finish(&state, actualHash);

    result.resize(32);
    for (unsigned int i = 0; i < 16; i++)
    {
      result[2 * i] = GetHexadecimalCharacter(static_cast<uint8_t>(actualHash[i] / 16));
      result[2 * i + 1] = GetHexadecimalCharacter(static_cast<uint8_t>(actualHash[i] % 16));
    }
  }
#endif


#if ORTHANC_ENABLE_BASE64 == 1
  void Toolbox::EncodeBase64(std::string& result, 
                             const std::string& data)
  {
    result = base64_encode(data);
  }

  void Toolbox::DecodeBase64(std::string& result, 
                             const std::string& data)
  {
    for (size_t i = 0; i < data.length(); i++)
    {
      if (!isalnum(data[i]) &&
          data[i] != '+' &&
          data[i] != '/' &&
          data[i] != '=')
      {
        // This is not a valid character for a Base64 string
        throw OrthancException(ErrorCode_BadFileFormat);
      }
    }

    result = base64_decode(data);
  }


  bool Toolbox::DecodeDataUriScheme(std::string& mime,
                                    std::string& content,
                                    const std::string& source)
  {
    boost::regex pattern("data:([^;]+);base64,([a-zA-Z0-9=+/]*)",
                         boost::regex::icase /* case insensitive search */);

    boost::cmatch what;
    if (regex_match(source.c_str(), what, pattern))
    {
      mime = what[1];
      DecodeBase64(content, what[2]);
      return true;
    }
    else
    {
      return false;
    }
  }


  void Toolbox::EncodeDataUriScheme(std::string& result,
                                    const std::string& mime,
                                    const std::string& content)
  {
    result = "data:" + mime + ";base64," + base64_encode(content);
  }

#endif


#if ORTHANC_ENABLE_LOCALE == 1
  static const char* GetBoostLocaleEncoding(const Encoding sourceEncoding)
  {
    switch (sourceEncoding)
    {
      case Encoding_Utf8:
        return "UTF-8";

      case Encoding_Ascii:
        return "ASCII";

      case Encoding_Latin1:
        return "ISO-8859-1";
        break;

      case Encoding_Latin2:
        return "ISO-8859-2";
        break;

      case Encoding_Latin3:
        return "ISO-8859-3";
        break;

      case Encoding_Latin4:
        return "ISO-8859-4";
        break;

      case Encoding_Latin5:
        return "ISO-8859-9";
        break;

      case Encoding_Cyrillic:
        return "ISO-8859-5";
        break;

      case Encoding_Windows1251:
        return "WINDOWS-1251";
        break;

      case Encoding_Arabic:
        return "ISO-8859-6";
        break;

      case Encoding_Greek:
        return "ISO-8859-7";
        break;

      case Encoding_Hebrew:
        return "ISO-8859-8";
        break;
        
      case Encoding_Japanese:
        return "SHIFT-JIS";
        break;

      case Encoding_Chinese:
        return "GB18030";
        break;

      case Encoding_Thai:
        return "TIS620.2533-0";
        break;

      default:
        throw OrthancException(ErrorCode_NotImplemented);
    }
  }
#endif


#if ORTHANC_ENABLE_LOCALE == 1
  std::string Toolbox::ConvertToUtf8(const std::string& source,
                                     Encoding sourceEncoding)
  {
    if (sourceEncoding == Encoding_Utf8)
    {
      // Already in UTF-8: No conversion is required
      return source;
    }

    if (sourceEncoding == Encoding_Ascii)
    {
      return ConvertToAscii(source);
    }

    const char* encoding = GetBoostLocaleEncoding(sourceEncoding);

    try
    {
      return boost::locale::conv::to_utf<char>(source, encoding);
    }
    catch (std::runtime_error&)
    {
      // Bad input string or bad encoding
      return ConvertToAscii(source);
    }
  }
#endif
  

#if ORTHANC_ENABLE_LOCALE == 1
  std::string Toolbox::ConvertFromUtf8(const std::string& source,
                                       Encoding targetEncoding)
  {
    if (targetEncoding == Encoding_Utf8)
    {
      // Already in UTF-8: No conversion is required
      return source;
    }

    if (targetEncoding == Encoding_Ascii)
    {
      return ConvertToAscii(source);
    }

    const char* encoding = GetBoostLocaleEncoding(targetEncoding);

    try
    {
      return boost::locale::conv::from_utf<char>(source, encoding);
    }
    catch (std::runtime_error&)
    {
      // Bad input string or bad encoding
      return ConvertToAscii(source);
    }
  }
#endif


  bool Toolbox::IsAsciiString(const void* data,
                              size_t size)
  {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(data);

    for (size_t i = 0; i < size; i++, p++)
    {
      if (*p > 127 || *p == 0 || iscntrl(*p))
      {
        return false;
      }
    }

    return true;
  }


  bool Toolbox::IsAsciiString(const std::string& s)
  {
    return IsAsciiString(s.c_str(), s.size());
  }
  

  std::string Toolbox::ConvertToAscii(const std::string& source)
  {
    std::string result;

    result.reserve(source.size() + 1);
    for (size_t i = 0; i < source.size(); i++)
    {
      if (source[i] <= 127 && source[i] >= 0 && !iscntrl(source[i]))
      {
        result.push_back(source[i]);
      }
    }

    return result;
  }


  void Toolbox::ComputeSHA1(std::string& result,
                            const void* data,
                            size_t size)
  {
    boost::uuids::detail::sha1 sha1;

    if (size > 0)
    {
      sha1.process_bytes(data, size);
    }

    unsigned int digest[5];

    // Sanity check for the memory layout: A SHA-1 digest is 160 bits wide
    assert(sizeof(unsigned int) == 4 && sizeof(digest) == (160 / 8)); 
    
    sha1.get_digest(digest);

    result.resize(8 * 5 + 4);
    sprintf(&result[0], "%08x-%08x-%08x-%08x-%08x",
            digest[0],
            digest[1],
            digest[2],
            digest[3],
            digest[4]);
  }

  void Toolbox::ComputeSHA1(std::string& result,
                            const std::string& data)
  {
    if (data.size() > 0)
    {
      ComputeSHA1(result, data.c_str(), data.size());
    }
    else
    {
      ComputeSHA1(result, NULL, 0);
    }
  }


  bool Toolbox::IsSHA1(const char* str,
                       size_t size)
  {
    if (size == 0)
    {
      return false;
    }

    const char* start = str;
    const char* end = str + size;

    // Trim the beginning of the string
    while (start < end)
    {
      if (*start == '\0' ||
          isspace(*start))
      {
        start++;
      }
      else
      {
        break;
      }
    }

    // Trim the trailing of the string
    while (start < end)
    {
      if (*(end - 1) == '\0' ||
          isspace(*(end - 1)))
      {
        end--;
      }
      else
      {
        break;
      }
    }

    if (end - start != 44)
    {
      return false;
    }

    for (unsigned int i = 0; i < 44; i++)
    {
      if (i == 8 ||
          i == 17 ||
          i == 26 ||
          i == 35)
      {
        if (start[i] != '-')
          return false;
      }
      else
      {
        if (!isalnum(start[i]))
          return false;
      }
    }

    return true;
  }


  bool Toolbox::IsSHA1(const std::string& s)
  {
    if (s.size() == 0)
    {
      return false;
    }
    else
    {
      return IsSHA1(s.c_str(), s.size());
    }
  }


  std::string Toolbox::StripSpaces(const std::string& source)
  {
    size_t first = 0;

    while (first < source.length() &&
           isspace(source[first]))
    {
      first++;
    }

    if (first == source.length())
    {
      // String containing only spaces
      return "";
    }

    size_t last = source.length();
    while (last > first &&
           isspace(source[last - 1]))
    {
      last--;
    }          
    
    assert(first <= last);
    return source.substr(first, last - first);
  }


  static char Hex2Dec(char c)
  {
    return ((c >= '0' && c <= '9') ? c - '0' :
            ((c >= 'a' && c <= 'f') ? c - 'a' + 10 : c - 'A' + 10));
  }

  void Toolbox::UrlDecode(std::string& s)
  {
    // http://en.wikipedia.org/wiki/Percent-encoding
    // http://www.w3schools.com/tags/ref_urlencode.asp
    // http://stackoverflow.com/questions/154536/encode-decode-urls-in-c

    if (s.size() == 0)
    {
      return;
    }

    size_t source = 0;
    size_t target = 0;

    while (source < s.size())
    {
      if (s[source] == '%' &&
          source + 2 < s.size() &&
          isalnum(s[source + 1]) &&
          isalnum(s[source + 2]))
      {
        s[target] = (Hex2Dec(s[source + 1]) << 4) | Hex2Dec(s[source + 2]);
        source += 3;
        target += 1;
      }
      else
      {
        if (s[source] == '+')
          s[target] = ' ';
        else
          s[target] = s[source];

        source++;
        target++;
      }
    }

    s.resize(target);
  }


  Endianness Toolbox::DetectEndianness()
  {
    // http://sourceforge.net/p/predef/wiki/Endianness/

    uint8_t buffer[4];

    buffer[0] = 0x00;
    buffer[1] = 0x01;
    buffer[2] = 0x02;
    buffer[3] = 0x03;

    switch (*((uint32_t *)buffer)) 
    {
      case 0x00010203: 
        return Endianness_Big;

      case 0x03020100: 
        return Endianness_Little;
        
      default:
        throw OrthancException(ErrorCode_NotImplemented);
    }
  }


  std::string Toolbox::WildcardToRegularExpression(const std::string& source)
  {
    // TODO - Speed up this with a regular expression

    std::string result = source;

    // Escape all special characters
    boost::replace_all(result, "\\", "\\\\");
    boost::replace_all(result, "^", "\\^");
    boost::replace_all(result, ".", "\\.");
    boost::replace_all(result, "$", "\\$");
    boost::replace_all(result, "|", "\\|");
    boost::replace_all(result, "(", "\\(");
    boost::replace_all(result, ")", "\\)");
    boost::replace_all(result, "[", "\\[");
    boost::replace_all(result, "]", "\\]");
    boost::replace_all(result, "+", "\\+");
    boost::replace_all(result, "/", "\\/");
    boost::replace_all(result, "{", "\\{");
    boost::replace_all(result, "}", "\\}");

    // Convert wildcards '*' and '?' to their regex equivalents
    boost::replace_all(result, "?", ".");
    boost::replace_all(result, "*", ".*");

    return result;
  }


  void Toolbox::TokenizeString(std::vector<std::string>& result,
                               const std::string& value,
                               char separator)
  {
    result.clear();

    std::string currentItem;

    for (size_t i = 0; i < value.size(); i++)
    {
      if (value[i] == separator)
      {
        result.push_back(currentItem);
        currentItem.clear();
      }
      else
      {
        currentItem.push_back(value[i]);
      }
    }

    result.push_back(currentItem);
  }


#if ORTHANC_ENABLE_PUGIXML == 1
  class ChunkedBufferWriter : public pugi::xml_writer
  {
  private:
    ChunkedBuffer buffer_;

  public:
    virtual void write(const void *data, size_t size)
    {
      if (size > 0)
      {
        buffer_.AddChunk(reinterpret_cast<const char*>(data), size);
      }
    }

    void Flatten(std::string& s)
    {
      buffer_.Flatten(s);
    }
  };


  static void JsonToXmlInternal(pugi::xml_node& target,
                                const Json::Value& source,
                                const std::string& arrayElement)
  {
    // http://jsoncpp.sourceforge.net/value_8h_source.html#l00030

    switch (source.type())
    {
      case Json::nullValue:
      {
        target.append_child(pugi::node_pcdata).set_value("null");
        break;
      }

      case Json::intValue:
      {
        std::string s = boost::lexical_cast<std::string>(source.asInt());
        target.append_child(pugi::node_pcdata).set_value(s.c_str());
        break;
      }

      case Json::uintValue:
      {
        std::string s = boost::lexical_cast<std::string>(source.asUInt());
        target.append_child(pugi::node_pcdata).set_value(s.c_str());
        break;
      }

      case Json::realValue:
      {
        std::string s = boost::lexical_cast<std::string>(source.asFloat());
        target.append_child(pugi::node_pcdata).set_value(s.c_str());
        break;
      }

      case Json::stringValue:
      {
        target.append_child(pugi::node_pcdata).set_value(source.asString().c_str());
        break;
      }

      case Json::booleanValue:
      {
        target.append_child(pugi::node_pcdata).set_value(source.asBool() ? "true" : "false");
        break;
      }

      case Json::arrayValue:
      {
        for (Json::Value::ArrayIndex i = 0; i < source.size(); i++)
        {
          pugi::xml_node node = target.append_child();
          node.set_name(arrayElement.c_str());
          JsonToXmlInternal(node, source[i], arrayElement);
        }
        break;
      }
        
      case Json::objectValue:
      {
        Json::Value::Members members = source.getMemberNames();

        for (size_t i = 0; i < members.size(); i++)
        {
          pugi::xml_node node = target.append_child();
          node.set_name(members[i].c_str());
          JsonToXmlInternal(node, source[members[i]], arrayElement);          
        }

        break;
      }

      default:
        throw OrthancException(ErrorCode_NotImplemented);
    }
  }


  void Toolbox::JsonToXml(std::string& target,
                          const Json::Value& source,
                          const std::string& rootElement,
                          const std::string& arrayElement)
  {
    pugi::xml_document doc;

    pugi::xml_node n = doc.append_child(rootElement.c_str());
    JsonToXmlInternal(n, source, arrayElement);

    pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
    decl.append_attribute("version").set_value("1.0");
    decl.append_attribute("encoding").set_value("utf-8");

    ChunkedBufferWriter writer;
    doc.save(writer, "  ", pugi::format_default, pugi::encoding_utf8);
    writer.Flatten(target);
  }

#endif


  
  bool Toolbox::IsInteger(const std::string& str)
  {
    std::string s = StripSpaces(str);

    if (s.size() == 0)
    {
      return false;
    }

    size_t pos = 0;
    if (s[0] == '-')
    {
      if (s.size() == 1)
      {
        return false;
      }

      pos = 1;
    }

    while (pos < s.size())
    {
      if (!isdigit(s[pos]))
      {
        return false;
      }

      pos++;
    }

    return true;
  }


  void Toolbox::CopyJsonWithoutComments(Json::Value& target,
                                        const Json::Value& source)
  {
    switch (source.type())
    {
      case Json::nullValue:
        target = Json::nullValue;
        break;

      case Json::intValue:
        target = source.asInt64();
        break;

      case Json::uintValue:
        target = source.asUInt64();
        break;

      case Json::realValue:
        target = source.asDouble();
        break;

      case Json::stringValue:
        target = source.asString();
        break;

      case Json::booleanValue:
        target = source.asBool();
        break;

      case Json::arrayValue:
      {
        target = Json::arrayValue;
        for (Json::Value::ArrayIndex i = 0; i < source.size(); i++)
        {
          Json::Value& item = target.append(Json::nullValue);
          CopyJsonWithoutComments(item, source[i]);
        }

        break;
      }

      case Json::objectValue:
      {
        target = Json::objectValue;
        Json::Value::Members members = source.getMemberNames();
        for (Json::Value::ArrayIndex i = 0; i < members.size(); i++)
        {
          const std::string item = members[i];
          CopyJsonWithoutComments(target[item], source[item]);
        }

        break;
      }

      default:
        break;
    }
  }


  bool Toolbox::StartsWith(const std::string& str,
                           const std::string& prefix)
  {
    if (str.size() < prefix.size())
    {
      return false;
    }
    else
    {
      return str.compare(0, prefix.size(), prefix) == 0;
    }
  }
  

  static bool IsUnreservedCharacter(char c)
  {
    // This function checks whether "c" is an unserved character
    // wrt. an URI percent-encoding
    // https://en.wikipedia.org/wiki/Percent-encoding#Percent-encoding%5Fin%5Fa%5FURI

    return ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' ||
            c == '_' ||
            c == '.' ||
            c == '~');
  }

  void Toolbox::UriEncode(std::string& target,
                          const std::string& source)
  {
    // Estimate the length of the percent-encoded URI
    size_t length = 0;

    for (size_t i = 0; i < source.size(); i++)
    {
      if (IsUnreservedCharacter(source[i]))
      {
        length += 1;
      }
      else
      {
        // This character must be percent-encoded
        length += 3;
      }
    }

    target.clear();
    target.reserve(length);

    for (size_t i = 0; i < source.size(); i++)
    {
      if (IsUnreservedCharacter(source[i]))
      {
        target.push_back(source[i]);
      }
      else
      {
        // This character must be percent-encoded
        uint8_t byte = static_cast<uint8_t>(source[i]);
        uint8_t a = byte >> 4;
        uint8_t b = byte & 0x0f;

        target.push_back('%');
        target.push_back(a < 10 ? a + '0' : a - 10 + 'A');
        target.push_back(b < 10 ? b + '0' : b - 10 + 'A');
      }
    }
  }


  static bool HasField(const Json::Value& json,
                       const std::string& key,
                       Json::ValueType expectedType)
  {
    if (json.type() != Json::objectValue ||
        !json.isMember(key))
    {
      return false;
    }
    else if (json[key].type() == expectedType)
    {
      return true;
    }
    else
    {
      throw OrthancException(ErrorCode_BadParameterType);
    }
  }


  std::string Toolbox::GetJsonStringField(const Json::Value& json,
                                          const std::string& key,
                                          const std::string& defaultValue)
  {
    if (HasField(json, key, Json::stringValue))
    {
      return json[key].asString();
    }
    else
    {
      return defaultValue;
    }
  }


  bool Toolbox::GetJsonBooleanField(const ::Json::Value& json,
                                    const std::string& key,
                                    bool defaultValue)
  {
    if (HasField(json, key, Json::booleanValue))
    {
      return json[key].asBool();
    }
    else
    {
      return defaultValue;
    }
  }


  int Toolbox::GetJsonIntegerField(const ::Json::Value& json,
                                   const std::string& key,
                                   int defaultValue)
  {
    if (HasField(json, key, Json::intValue))
    {
      return json[key].asInt();
    }
    else
    {
      return defaultValue;
    }
  }


  unsigned int Toolbox::GetJsonUnsignedIntegerField(const ::Json::Value& json,
                                                    const std::string& key,
                                                    unsigned int defaultValue)
  {
    int v = GetJsonIntegerField(json, key, defaultValue);

    if (v < 0)
    {
      throw OrthancException(ErrorCode_ParameterOutOfRange);
    }
    else
    {
      return static_cast<unsigned int>(v);
    }
  }


  bool Toolbox::IsUuid(const std::string& str)
  {
    if (str.size() != 36)
    {
      return false;
    }

    for (size_t i = 0; i < str.length(); i++)
    {
      if (i == 8 || i == 13 || i == 18 || i == 23)
      {
        if (str[i] != '-')
          return false;
      }
      else
      {
        if (!isalnum(str[i]))
          return false;
      }
    }

    return true;
  }


  bool Toolbox::StartsWithUuid(const std::string& str)
  {
    if (str.size() < 36)
    {
      return false;
    }

    if (str.size() == 36)
    {
      return IsUuid(str);
    }

    assert(str.size() > 36);
    if (!isspace(str[36]))
    {
      return false;
    }

    return IsUuid(str.substr(0, 36));
  }


#if ORTHANC_ENABLE_LOCALE == 1
  static std::auto_ptr<std::locale>  globalLocale_;

  static bool SetGlobalLocale(const char* locale)
  {
    globalLocale_.reset(NULL);

    try
    {
      if (locale == NULL)
      {
        LOG(WARNING) << "Falling back to system-wide default locale";
        globalLocale_.reset(new std::locale());
      }
      else
      {
        LOG(INFO) << "Using locale: \"" << locale << "\" for case-insensitive comparison of strings";
        globalLocale_.reset(new std::locale(locale));
      }
    }
    catch (std::runtime_error&)
    {
    }

    return (globalLocale_.get() != NULL);
  }
  
  void Toolbox::InitializeGlobalLocale(const char* locale)
  {
    // Make Orthanc use English, United States locale
    // Linux: use "en_US.UTF-8"
    // Windows: use ""
    // Wine: use NULL
    
#if defined(__MINGW32__)
    // Visibly, there is no support of locales in MinGW yet
    // http://mingw.5.n7.nabble.com/How-to-use-std-locale-global-with-MinGW-correct-td33048.html
    static const char* DEFAULT_LOCALE = NULL;
#elif defined(_WIN32)
    // For Windows: use default locale (using "en_US" does not work)
    static const char* DEFAULT_LOCALE = "";
#else
    // For Linux & cie
    static const char* DEFAULT_LOCALE = "en_US.UTF-8";
#endif

    bool ok;
    
    if (locale == NULL)
    {
      ok = SetGlobalLocale(DEFAULT_LOCALE);

#if defined(__MINGW32__)
      LOG(WARNING) << "This is a MinGW build, case-insensitive comparison of "
                   << "strings with accents will not work outside of Wine";
#endif
    }
    else
    {
      ok = SetGlobalLocale(locale);
    }

    if (!ok &&
        !SetGlobalLocale(NULL))
    {
      LOG(ERROR) << "Cannot initialize global locale";
      throw OrthancException(ErrorCode_InternalError);
    }

  }


  void Toolbox::FinalizeGlobalLocale()
  {
    globalLocale_.reset();
  }

  
  std::string Toolbox::ToUpperCaseWithAccents(const std::string& source)
  {
    if (globalLocale_.get() == NULL)
    {
      LOG(ERROR) << "No global locale was set, call Toolbox::InitializeGlobalLocale()";
      throw OrthancException(ErrorCode_BadSequenceOfCalls);
    }

    /**
     * A few notes about locales:
     *
     * (1) We don't use "case folding":
     * http://www.boost.org/doc/libs/1_64_0/libs/locale/doc/html/conversions.html
     *
     * Characters are made uppercase one by one. This is because, in
     * static builds, we are using iconv, which is visibly not
     * supported correctly (TODO: Understand why). Case folding seems
     * to be working correctly if using the default backend under
     * Linux (ICU or POSIX?). If one wishes to use case folding, one
     * would use:
     *
     *   boost::locale::generator gen;
     *   std::locale::global(gen(DEFAULT_LOCALE));
     *   return boost::locale::to_upper(source);
     *
     * (2) The function "boost::algorithm::to_upper_copy" does not
     * make use of the "std::locale::global()". We therefore create a
     * global variable "globalLocale_".
     * 
     * (3) The variant of "boost::algorithm::to_upper_copy()" that
     * uses std::string does not work properly. We need to apply it
     * one wide strings (std::wstring). This explains the two calls to
     * "utf_to_utf" in order to convert to/from std::wstring.
     **/

    std::wstring w = boost::locale::conv::utf_to_utf<wchar_t>(source);
    w = boost::algorithm::to_upper_copy<std::wstring>(w, *globalLocale_);
    return boost::locale::conv::utf_to_utf<char>(w);
  }
#endif


  std::string Toolbox::GenerateUuid()
  {
#ifdef WIN32
    UUID uuid;
    UuidCreate ( &uuid );

    unsigned char * str;
    UuidToStringA ( &uuid, &str );

    std::string s( ( char* ) str );

    RpcStringFreeA ( &str );
#else
    uuid_t uuid;
    uuid_generate_random ( uuid );
    char s[37];
    uuid_unparse ( uuid, s );
#endif
    return s;
  }
}



OrthancLinesIterator* OrthancLinesIterator_Create(const std::string& content)
{
  return reinterpret_cast<OrthancLinesIterator*>(new Orthanc::Toolbox::LinesIterator(content));
}


bool OrthancLinesIterator_GetLine(std::string& target,
                                         const OrthancLinesIterator* iterator)
{
  if (iterator != NULL)
  {
    return reinterpret_cast<const Orthanc::Toolbox::LinesIterator*>(iterator)->GetLine(target);
  }
  else
  {
    return false;
  }
}


void OrthancLinesIterator_Next(OrthancLinesIterator* iterator)
{
  if (iterator != NULL)
  {
    reinterpret_cast<Orthanc::Toolbox::LinesIterator*>(iterator)->Next();
  }
}


void OrthancLinesIterator_Free(OrthancLinesIterator* iterator)
{
  if (iterator != NULL)
  {
    delete reinterpret_cast<const Orthanc::Toolbox::LinesIterator*>(iterator);
  }
}
