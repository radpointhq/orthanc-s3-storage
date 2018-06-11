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

#if !defined(HAS_ORTHANC_EXCEPTION)
#  error The macro HAS_ORTHANC_EXCEPTION must be defined
#endif


#if HAS_ORTHANC_EXCEPTION == 1
#  include "../../../Core/OrthancException.h"
#  define ORTHANC_PLUGINS_ERROR_ENUMERATION     ::Orthanc::ErrorCode
#  define ORTHANC_PLUGINS_EXCEPTION_CLASS       ::Orthanc::OrthancException
#  define ORTHANC_PLUGINS_GET_ERROR_CODE(code)  ::Orthanc::ErrorCode_ ## code
#else
#  include <orthanc/OrthancCPlugin.h>
#  define ORTHANC_PLUGINS_ERROR_ENUMERATION     ::OrthancPluginErrorCode
#  define ORTHANC_PLUGINS_EXCEPTION_CLASS       ::OrthancPlugins::PluginException
#  define ORTHANC_PLUGINS_GET_ERROR_CODE(code)  ::OrthancPluginErrorCode_ ## code
#endif


#define ORTHANC_PLUGINS_THROW_PLUGIN_ERROR_CODE(code)                   \
  throw ORTHANC_PLUGINS_EXCEPTION_CLASS(static_cast<ORTHANC_PLUGINS_ERROR_ENUMERATION>(code));


#define ORTHANC_PLUGINS_THROW_EXCEPTION(code)                           \
  throw ORTHANC_PLUGINS_EXCEPTION_CLASS(ORTHANC_PLUGINS_GET_ERROR_CODE(code));
                                                  

#define ORTHANC_PLUGINS_CHECK_ERROR(code)                           \
  if (code != ORTHANC_PLUGINS_GET_ERROR_CODE(Success))              \
  {                                                                 \
    ORTHANC_PLUGINS_THROW_EXCEPTION(code);                          \
  }


namespace OrthancPlugins
{
#if HAS_ORTHANC_EXCEPTION == 0
  class PluginException
  {
  private:
    OrthancPluginErrorCode  code_;

  public:
    explicit PluginException(OrthancPluginErrorCode code) : code_(code)
    {
    }

    OrthancPluginErrorCode GetErrorCode() const
    {
      return code_;
    }

    const char* What(OrthancPluginContext* context) const
    {
      const char* description = OrthancPluginGetErrorDescription(context, code_);
      if (description)
      {
        return description;
      }
      else
      {
        return "No description available";
      }
    }
  };
#endif
}
