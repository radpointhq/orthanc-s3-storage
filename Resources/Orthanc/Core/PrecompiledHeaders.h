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

#if defined(_WIN32) && !defined(NOMINMAX)
#define NOMINMAX
#endif

#if ORTHANC_USE_PRECOMPILED_HEADERS == 1

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/locale.hpp>
#include <boost/regex.hpp>
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <json/value.h>

#if ORTHANC_ENABLE_PUGIXML == 1
#  include <pugixml.hpp>
#endif

#include "Enumerations.h"
#include "Logging.h"
#include "OrthancException.h"
#include "Toolbox.h"

#if ORTHANC_ENABLE_DCMTK == 1
// Headers from DCMTK used in Orthanc headers 
#  include <dcmtk/dcmdata/dcdatset.h>
#  include <dcmtk/dcmdata/dcfilefo.h>
#  include <dcmtk/dcmdata/dcmetinf.h>
#  include <dcmtk/dcmdata/dcpixseq.h>
#endif

#if ORTHANC_ENABLE_DCMTK_NETWORKING == 1
#  include "DicomNetworking/DicomServer.h"

// Headers from DCMTK used in Orthanc headers 
#  include <dcmtk/dcmnet/dimse.h>
#endif

#endif
