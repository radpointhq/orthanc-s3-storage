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

#include <iostream>

#if !defined(ORTHANC_ENABLE_LOGGING)
#  error The macro ORTHANC_ENABLE_LOGGING must be defined
#endif

#if !defined(ORTHANC_ENABLE_LOGGING_PLUGIN)
#  if ORTHANC_ENABLE_LOGGING == 1
#    error The macro ORTHANC_ENABLE_LOGGING_PLUGIN must be defined
#  else
#    define ORTHANC_ENABLE_LOGGING_PLUGIN 0
#  endif
#endif

#if !defined(ORTHANC_ENABLE_LOGGING_STDIO)
#  if ORTHANC_ENABLE_LOGGING == 1
#    error The macro ORTHANC_ENABLE_LOGGING_STDIO must be defined
#  else
#    define ORTHANC_ENABLE_LOGGING_STDIO 0
#  endif
#endif

#if ORTHANC_ENABLE_LOGGING_PLUGIN == 1
#  include <orthanc/OrthancCPlugin.h>
#endif

#include <boost/lexical_cast.hpp>

namespace Orthanc
{
  namespace Logging
  {
#if ORTHANC_ENABLE_LOGGING_PLUGIN == 1
    void Initialize(OrthancPluginContext* context);
#else
    void Initialize();
#endif

    void Finalize();

    void Reset();

    void Flush();

    void EnableInfoLevel(bool enabled);

    void EnableTraceLevel(bool enabled);

    void SetTargetFile(const std::string& path);

    void SetTargetFolder(const std::string& path);

    struct NullStream : public std::ostream 
    {
      NullStream() : 
        std::ios(0), 
        std::ostream(0)
      {
      }
      
      template <typename T>
      std::ostream& operator<< (const T& message)
      {
        return *this;
      }
    };
  }
}


#if ORTHANC_ENABLE_LOGGING != 1

#  define LOG(level)   ::Orthanc::Logging::NullStream()
#  define VLOG(level)  ::Orthanc::Logging::NullStream()


#elif (ORTHANC_ENABLE_LOGGING_PLUGIN == 1 ||    \
       ORTHANC_ENABLE_LOGGING_STDIO == 1)

#  include <boost/noncopyable.hpp>
#  define LOG(level)  ::Orthanc::Logging::InternalLogger \
  (::Orthanc::Logging::InternalLevel_ ## level, __FILE__, __LINE__)
#  define VLOG(level) ::Orthanc::Logging::InternalLogger \
  (::Orthanc::Logging::InternalLevel_TRACE, __FILE__, __LINE__)

namespace Orthanc
{
  namespace Logging
  {
    enum InternalLevel
    {
      InternalLevel_ERROR,
      InternalLevel_WARNING,
      InternalLevel_INFO,
      InternalLevel_TRACE
    };
    
    class InternalLogger : public boost::noncopyable
    {
    private:
      InternalLevel  level_;
      std::string    message_;

    public:
      InternalLogger(InternalLevel level,
                     const char* file,
                     int line);

      ~InternalLogger();
      
      template <typename T>
      InternalLogger& operator<< (const T& message)
      {
        message_ += boost::lexical_cast<std::string>(message);
        return *this;
      }
    };
  }
}




#else  /* ORTHANC_ENABLE_LOGGING_PLUGIN == 0 && 
          ORTHANC_ENABLE_LOGGING_STDIO == 0 && 
          ORTHANC_ENABLE_LOGGING == 1 */

#  include <boost/thread/mutex.hpp>
#  define LOG(level)  ::Orthanc::Logging::InternalLogger(#level,  __FILE__, __LINE__)
#  define VLOG(level) ::Orthanc::Logging::InternalLogger("TRACE", __FILE__, __LINE__)

namespace Orthanc
{
  namespace Logging
  {
    class InternalLogger
    {
    private:
      boost::mutex::scoped_lock lock_;
      NullStream                null_;
      std::ostream*             stream_;

    public:
      InternalLogger(const char* level,
                     const char* file,
                     int line);

      ~InternalLogger();
      
      template <typename T>
      std::ostream& operator<< (const T& message)
      {
        return (*stream_) << boost::lexical_cast<std::string>(message);
      }
    };
  }
}

#endif  // ORTHANC_ENABLE_LOGGING
