/**
 * @ingroup CInterface
 **/

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

#include "OrthancCPlugin.h"


/** @{ */

#ifdef __cplusplus
extern "C"
{
#endif


  /**
   * Opaque structure that represents the context of a custom database engine.
   * @ingroup Callbacks
   **/
  typedef struct _OrthancPluginDatabaseContext_t OrthancPluginDatabaseContext;


/*<! @cond Doxygen_Suppress */
  typedef enum
  {
    _OrthancPluginDatabaseAnswerType_None = 0,

    /* Events */
    _OrthancPluginDatabaseAnswerType_DeletedAttachment = 1,
    _OrthancPluginDatabaseAnswerType_DeletedResource = 2,
    _OrthancPluginDatabaseAnswerType_RemainingAncestor = 3,

    /* Return value */
    _OrthancPluginDatabaseAnswerType_Attachment = 10,
    _OrthancPluginDatabaseAnswerType_Change = 11,
    _OrthancPluginDatabaseAnswerType_DicomTag = 12,
    _OrthancPluginDatabaseAnswerType_ExportedResource = 13,
    _OrthancPluginDatabaseAnswerType_Int32 = 14,
    _OrthancPluginDatabaseAnswerType_Int64 = 15,
    _OrthancPluginDatabaseAnswerType_Resource = 16,
    _OrthancPluginDatabaseAnswerType_String = 17,

    _OrthancPluginDatabaseAnswerType_INTERNAL = 0x7fffffff
  } _OrthancPluginDatabaseAnswerType;


  typedef struct
  {
    const char* uuid;
    int32_t     contentType;
    uint64_t    uncompressedSize;
    const char* uncompressedHash;
    int32_t     compressionType;
    uint64_t    compressedSize;
    const char* compressedHash;
  } OrthancPluginAttachment;

  typedef struct
  {
    uint16_t     group;
    uint16_t     element;
    const char*  value;
  } OrthancPluginDicomTag;

  typedef struct
  {
    int64_t                    seq;
    int32_t                    changeType;
    OrthancPluginResourceType  resourceType;
    const char*                publicId;
    const char*                date;
  } OrthancPluginChange;

  typedef struct
  {
    int64_t                    seq;
    OrthancPluginResourceType  resourceType;
    const char*                publicId;
    const char*                modality;
    const char*                date;
    const char*                patientId;
    const char*                studyInstanceUid;
    const char*                seriesInstanceUid;
    const char*                sopInstanceUid;
  } OrthancPluginExportedResource;


  typedef struct
  {
    OrthancPluginDatabaseContext* database;
    _OrthancPluginDatabaseAnswerType  type;
    int32_t      valueInt32;
    uint32_t     valueUint32;
    int64_t      valueInt64;
    const char  *valueString;
    const void  *valueGeneric;
  } _OrthancPluginDatabaseAnswer;

  ORTHANC_PLUGIN_INLINE void OrthancPluginDatabaseAnswerString(
    OrthancPluginContext*          context,
    OrthancPluginDatabaseContext*  database,
    const char*                    value)
  {
    _OrthancPluginDatabaseAnswer params;
    memset(&params, 0, sizeof(params));
    params.database = database;
    params.type = _OrthancPluginDatabaseAnswerType_String;
    params.valueString = value;
    context->InvokeService(context, _OrthancPluginService_DatabaseAnswer, &params);
  }

  ORTHANC_PLUGIN_INLINE void OrthancPluginDatabaseAnswerChange(
    OrthancPluginContext*          context,
    OrthancPluginDatabaseContext*  database,
    const OrthancPluginChange*     change)
  {
    _OrthancPluginDatabaseAnswer params;
    memset(&params, 0, sizeof(params));

    params.database = database;
    params.type = _OrthancPluginDatabaseAnswerType_Change;
    params.valueUint32 = 0;
    params.valueGeneric = change;

    context->InvokeService(context, _OrthancPluginService_DatabaseAnswer, &params);
  }

  ORTHANC_PLUGIN_INLINE void OrthancPluginDatabaseAnswerChangesDone(
    OrthancPluginContext*          context,
    OrthancPluginDatabaseContext*  database)
  {
    _OrthancPluginDatabaseAnswer params;
    memset(&params, 0, sizeof(params));

    params.database = database;
    params.type = _OrthancPluginDatabaseAnswerType_Change;
    params.valueUint32 = 1;
    params.valueGeneric = NULL;

    context->InvokeService(context, _OrthancPluginService_DatabaseAnswer, &params);
  }

  ORTHANC_PLUGIN_INLINE void OrthancPluginDatabaseAnswerInt32(
    OrthancPluginContext*          context,
    OrthancPluginDatabaseContext*  database,
    int32_t                        value)
  {
    _OrthancPluginDatabaseAnswer params;
    memset(&params, 0, sizeof(params));
    params.database = database;
    params.type = _OrthancPluginDatabaseAnswerType_Int32;
    params.valueInt32 = value;
    context->InvokeService(context, _OrthancPluginService_DatabaseAnswer, &params);
  }

  ORTHANC_PLUGIN_INLINE void OrthancPluginDatabaseAnswerInt64(
    OrthancPluginContext*          context,
    OrthancPluginDatabaseContext*  database,
    int64_t                        value)
  {
    _OrthancPluginDatabaseAnswer params;
    memset(&params, 0, sizeof(params));
    params.database = database;
    params.type = _OrthancPluginDatabaseAnswerType_Int64;
    params.valueInt64 = value;
    context->InvokeService(context, _OrthancPluginService_DatabaseAnswer, &params);
  }

  ORTHANC_PLUGIN_INLINE void OrthancPluginDatabaseAnswerExportedResource(
    OrthancPluginContext*                 context,
    OrthancPluginDatabaseContext*         database,
    const OrthancPluginExportedResource*  exported)
  {
    _OrthancPluginDatabaseAnswer params;
    memset(&params, 0, sizeof(params));

    params.database = database;
    params.type = _OrthancPluginDatabaseAnswerType_ExportedResource;
    params.valueUint32 = 0;
    params.valueGeneric = exported;
    context->InvokeService(context, _OrthancPluginService_DatabaseAnswer, &params);
  }

  ORTHANC_PLUGIN_INLINE void OrthancPluginDatabaseAnswerExportedResourcesDone(
    OrthancPluginContext*                 context,
    OrthancPluginDatabaseContext*         database)
  {
    _OrthancPluginDatabaseAnswer params;
    memset(&params, 0, sizeof(params));

    params.database = database;
    params.type = _OrthancPluginDatabaseAnswerType_ExportedResource;
    params.valueUint32 = 1;
    params.valueGeneric = NULL;
    context->InvokeService(context, _OrthancPluginService_DatabaseAnswer, &params);
  }

  ORTHANC_PLUGIN_INLINE void OrthancPluginDatabaseAnswerDicomTag(
    OrthancPluginContext*          context,
    OrthancPluginDatabaseContext*  database,
    const OrthancPluginDicomTag*   tag)
  {
    _OrthancPluginDatabaseAnswer params;
    memset(&params, 0, sizeof(params));
    params.database = database;
    params.type = _OrthancPluginDatabaseAnswerType_DicomTag;
    params.valueGeneric = tag;
    context->InvokeService(context, _OrthancPluginService_DatabaseAnswer, &params);
  }

  ORTHANC_PLUGIN_INLINE void OrthancPluginDatabaseAnswerAttachment(
    OrthancPluginContext*          context,
    OrthancPluginDatabaseContext*  database,
    const OrthancPluginAttachment* attachment)
  {
    _OrthancPluginDatabaseAnswer params;
    memset(&params, 0, sizeof(params));
    params.database = database;
    params.type = _OrthancPluginDatabaseAnswerType_Attachment;
    params.valueGeneric = attachment;
    context->InvokeService(context, _OrthancPluginService_DatabaseAnswer, &params);
  }

  ORTHANC_PLUGIN_INLINE void OrthancPluginDatabaseAnswerResource(
    OrthancPluginContext*          context,
    OrthancPluginDatabaseContext*  database,
    int64_t                        id,
    OrthancPluginResourceType      resourceType)
  {
    _OrthancPluginDatabaseAnswer params;
    memset(&params, 0, sizeof(params));
    params.database = database;
    params.type = _OrthancPluginDatabaseAnswerType_Resource;
    params.valueInt64 = id;
    params.valueInt32 = (int32_t) resourceType;
    context->InvokeService(context, _OrthancPluginService_DatabaseAnswer, &params);
  }

  ORTHANC_PLUGIN_INLINE void OrthancPluginDatabaseSignalDeletedAttachment(
    OrthancPluginContext*          context,
    OrthancPluginDatabaseContext*  database,
    const OrthancPluginAttachment* attachment)
  {
    _OrthancPluginDatabaseAnswer params;
    memset(&params, 0, sizeof(params));
    params.database = database;
    params.type = _OrthancPluginDatabaseAnswerType_DeletedAttachment;
    params.valueGeneric = attachment;
    context->InvokeService(context, _OrthancPluginService_DatabaseAnswer, &params);
  }

  ORTHANC_PLUGIN_INLINE void OrthancPluginDatabaseSignalDeletedResource(
    OrthancPluginContext*          context,
    OrthancPluginDatabaseContext*  database,
    const char*                    publicId,
    OrthancPluginResourceType      resourceType)
  {
    _OrthancPluginDatabaseAnswer params;
    memset(&params, 0, sizeof(params));
    params.database = database;
    params.type = _OrthancPluginDatabaseAnswerType_DeletedResource;
    params.valueString = publicId;
    params.valueInt32 = (int32_t) resourceType;
    context->InvokeService(context, _OrthancPluginService_DatabaseAnswer, &params);
  }

  ORTHANC_PLUGIN_INLINE void OrthancPluginDatabaseSignalRemainingAncestor(
    OrthancPluginContext*          context,
    OrthancPluginDatabaseContext*  database,
    const char*                    ancestorId,
    OrthancPluginResourceType      ancestorType)
  {
    _OrthancPluginDatabaseAnswer params;
    memset(&params, 0, sizeof(params));
    params.database = database;
    params.type = _OrthancPluginDatabaseAnswerType_RemainingAncestor;
    params.valueString = ancestorId;
    params.valueInt32 = (int32_t) ancestorType;
    context->InvokeService(context, _OrthancPluginService_DatabaseAnswer, &params);
  }





  typedef struct
  {
    OrthancPluginErrorCode  (*addAttachment) (
      /* inputs */
      void* payload,
      int64_t id,
      const OrthancPluginAttachment* attachment);
                             
    OrthancPluginErrorCode  (*attachChild) (
      /* inputs */
      void* payload,
      int64_t parent,
      int64_t child);
                             
    OrthancPluginErrorCode  (*clearChanges) (
      /* inputs */
      void* payload);
                             
    OrthancPluginErrorCode  (*clearExportedResources) (
      /* inputs */
      void* payload);

    OrthancPluginErrorCode  (*createResource) (
      /* outputs */
      int64_t* id, 
      /* inputs */
      void* payload,
      const char* publicId,
      OrthancPluginResourceType resourceType);           
                   
    OrthancPluginErrorCode  (*deleteAttachment) (
      /* inputs */
      void* payload,
      int64_t id,
      int32_t contentType);
   
    OrthancPluginErrorCode  (*deleteMetadata) (
      /* inputs */
      void* payload,
      int64_t id,
      int32_t metadataType);
   
    OrthancPluginErrorCode  (*deleteResource) (
      /* inputs */
      void* payload,
      int64_t id);    

    /* Output: Use OrthancPluginDatabaseAnswerString() */
    OrthancPluginErrorCode  (*getAllPublicIds) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      OrthancPluginResourceType resourceType);

    /* Output: Use OrthancPluginDatabaseAnswerChange() and
     * OrthancPluginDatabaseAnswerChangesDone() */
    OrthancPluginErrorCode  (*getChanges) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      int64_t since,
      uint32_t maxResult);

    /* Output: Use OrthancPluginDatabaseAnswerInt64() */
    OrthancPluginErrorCode  (*getChildrenInternalId) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      int64_t id);
                   
    /* Output: Use OrthancPluginDatabaseAnswerString() */
    OrthancPluginErrorCode  (*getChildrenPublicId) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      int64_t id);

    /* Output: Use OrthancPluginDatabaseAnswerExportedResource() and
     * OrthancPluginDatabaseAnswerExportedResourcesDone() */
    OrthancPluginErrorCode  (*getExportedResources) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      int64_t  since,
      uint32_t  maxResult);
                   
    /* Output: Use OrthancPluginDatabaseAnswerChange() */
    OrthancPluginErrorCode  (*getLastChange) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload);

    /* Output: Use OrthancPluginDatabaseAnswerExportedResource() */
    OrthancPluginErrorCode  (*getLastExportedResource) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload);
                   
    /* Output: Use OrthancPluginDatabaseAnswerDicomTag() */
    OrthancPluginErrorCode  (*getMainDicomTags) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      int64_t id);
                   
    /* Output: Use OrthancPluginDatabaseAnswerString() */
    OrthancPluginErrorCode  (*getPublicId) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      int64_t id);

    OrthancPluginErrorCode  (*getResourceCount) (
      /* outputs */
      uint64_t* target,
      /* inputs */
      void* payload,
      OrthancPluginResourceType  resourceType);
                   
    OrthancPluginErrorCode  (*getResourceType) (
      /* outputs */
      OrthancPluginResourceType* resourceType,
      /* inputs */
      void* payload,
      int64_t id);

    OrthancPluginErrorCode  (*getTotalCompressedSize) (
      /* outputs */
      uint64_t* target,
      /* inputs */
      void* payload);
                   
    OrthancPluginErrorCode  (*getTotalUncompressedSize) (
      /* outputs */
      uint64_t* target,
      /* inputs */
      void* payload);
                   
    OrthancPluginErrorCode  (*isExistingResource) (
      /* outputs */
      int32_t* existing,
      /* inputs */
      void* payload,
      int64_t id);

    OrthancPluginErrorCode  (*isProtectedPatient) (
      /* outputs */
      int32_t* isProtected,
      /* inputs */
      void* payload,
      int64_t id);

    /* Output: Use OrthancPluginDatabaseAnswerInt32() */
    OrthancPluginErrorCode  (*listAvailableMetadata) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      int64_t id);
                   
    /* Output: Use OrthancPluginDatabaseAnswerInt32() */
    OrthancPluginErrorCode  (*listAvailableAttachments) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      int64_t id);

    OrthancPluginErrorCode  (*logChange) (
      /* inputs */
      void* payload,
      const OrthancPluginChange* change);
                   
    OrthancPluginErrorCode  (*logExportedResource) (
      /* inputs */
      void* payload,
      const OrthancPluginExportedResource* exported);
                   
    /* Output: Use OrthancPluginDatabaseAnswerAttachment() */
    OrthancPluginErrorCode  (*lookupAttachment) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      int64_t id,
      int32_t contentType);

    /* Output: Use OrthancPluginDatabaseAnswerString() */
    OrthancPluginErrorCode  (*lookupGlobalProperty) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      int32_t property);

    /* Use "OrthancPluginDatabaseExtensions::lookupIdentifier3" 
       instead of this function as of Orthanc 0.9.5 (db v6), can be set to NULL.
       Output: Use OrthancPluginDatabaseAnswerInt64() */
    OrthancPluginErrorCode  (*lookupIdentifier) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      const OrthancPluginDicomTag* tag);

    /* Unused starting with Orthanc 0.9.5 (db v6), can be set to NULL.
       Output: Use OrthancPluginDatabaseAnswerInt64() */
    OrthancPluginErrorCode  (*lookupIdentifier2) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      const char* value);

    /* Output: Use OrthancPluginDatabaseAnswerString() */
    OrthancPluginErrorCode  (*lookupMetadata) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      int64_t id,
      int32_t metadata);

    /* Output: Use OrthancPluginDatabaseAnswerInt64() */
    OrthancPluginErrorCode  (*lookupParent) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      int64_t id);

    /* Output: Use OrthancPluginDatabaseAnswerResource() */
    OrthancPluginErrorCode  (*lookupResource) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      const char* publicId);

    /* Output: Use OrthancPluginDatabaseAnswerInt64() */
    OrthancPluginErrorCode  (*selectPatientToRecycle) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload);

    /* Output: Use OrthancPluginDatabaseAnswerInt64() */
    OrthancPluginErrorCode  (*selectPatientToRecycle2) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      int64_t patientIdToAvoid);

    OrthancPluginErrorCode  (*setGlobalProperty) (
      /* inputs */
      void* payload,
      int32_t property,
      const char* value);

    OrthancPluginErrorCode  (*setMainDicomTag) (
      /* inputs */
      void* payload,
      int64_t id,
      const OrthancPluginDicomTag* tag);

    OrthancPluginErrorCode  (*setIdentifierTag) (
      /* inputs */
      void* payload,
      int64_t id,
      const OrthancPluginDicomTag* tag);

    OrthancPluginErrorCode  (*setMetadata) (
      /* inputs */
      void* payload,
      int64_t id,
      int32_t metadata,
      const char* value);

    OrthancPluginErrorCode  (*setProtectedPatient) (
      /* inputs */
      void* payload,
      int64_t id,
      int32_t isProtected);

    OrthancPluginErrorCode  (*startTransaction) (
      /* inputs */
      void* payload);

    OrthancPluginErrorCode  (*rollbackTransaction) (
      /* inputs */
      void* payload);

    OrthancPluginErrorCode  (*commitTransaction) (
      /* inputs */
      void* payload);

    OrthancPluginErrorCode  (*open) (
      /* inputs */
      void* payload);

    OrthancPluginErrorCode  (*close) (
      /* inputs */
      void* payload);

  } OrthancPluginDatabaseBackend;


  typedef struct
  {
    /* Output: Use OrthancPluginDatabaseAnswerString() */
    OrthancPluginErrorCode  (*getAllPublicIdsWithLimit) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      OrthancPluginResourceType resourceType,
      uint64_t since,
      uint64_t limit);

    OrthancPluginErrorCode  (*getDatabaseVersion) (
      /* outputs */
      uint32_t* version,
      /* inputs */
      void* payload);

    OrthancPluginErrorCode  (*upgradeDatabase) (
      /* inputs */
      void* payload,
      uint32_t targetVersion,
      OrthancPluginStorageArea* storageArea);
 
    OrthancPluginErrorCode  (*clearMainDicomTags) (
      /* inputs */
      void* payload,
      int64_t id);

    /* Output: Use OrthancPluginDatabaseAnswerInt64() */
    OrthancPluginErrorCode  (*getAllInternalIds) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      OrthancPluginResourceType resourceType);

    /* Output: Use OrthancPluginDatabaseAnswerInt64() */
    OrthancPluginErrorCode  (*lookupIdentifier3) (
      /* outputs */
      OrthancPluginDatabaseContext* context,
      /* inputs */
      void* payload,
      OrthancPluginResourceType resourceType,
      const OrthancPluginDicomTag* tag,
      OrthancPluginIdentifierConstraint constraint);
   } OrthancPluginDatabaseExtensions;

/*<! @endcond */


  typedef struct
  {
    OrthancPluginDatabaseContext**       result;
    const OrthancPluginDatabaseBackend*  backend;
    void*                                payload;
  } _OrthancPluginRegisterDatabaseBackend;

  /**
   * Register a custom database back-end.
   *
   * Instead of manually filling the OrthancPluginDatabaseBackend
   * structure, you should instead implement a concrete C++ class
   * deriving from ::OrthancPlugins::IDatabaseBackend, and register it
   * using ::OrthancPlugins::DatabaseBackendAdapter::Register().
   * 
   * @param context The Orthanc plugin context, as received by OrthancPluginInitialize().
   * @param backend The callbacks of the custom database engine.
   * @param payload Pointer containing private information for the database engine.
   * @return The context of the database engine (it must not be manually freed).
   * @ingroup Callbacks
   * @deprecated
   * @see OrthancPluginRegisterDatabaseBackendV2
   **/
  ORTHANC_PLUGIN_INLINE OrthancPluginDatabaseContext* OrthancPluginRegisterDatabaseBackend(
    OrthancPluginContext*                context,
    const OrthancPluginDatabaseBackend*  backend,
    void*                                payload)
  {
    OrthancPluginDatabaseContext* result = NULL;
    _OrthancPluginRegisterDatabaseBackend params;

    if (sizeof(int32_t) != sizeof(_OrthancPluginDatabaseAnswerType))
    {
      return NULL;
    }

    memset(&params, 0, sizeof(params));
    params.backend = backend;
    params.result = &result;
    params.payload = payload;

    if (context->InvokeService(context, _OrthancPluginService_RegisterDatabaseBackend, &params) ||
        result == NULL)
    {
      /* Error */
      return NULL;
    }
    else
    {
      return result;
    }
  }


  typedef struct
  {
    OrthancPluginDatabaseContext**          result;
    const OrthancPluginDatabaseBackend*     backend;
    void*                                   payload;
    const OrthancPluginDatabaseExtensions*  extensions;
    uint32_t                                extensionsSize;
  } _OrthancPluginRegisterDatabaseBackendV2;


  /**
   * Register a custom database back-end.
   *
   * Instead of manually filling the OrthancPluginDatabaseBackendV2
   * structure, you should instead implement a concrete C++ class
   * deriving from ::OrthancPlugins::IDatabaseBackend, and register it
   * using ::OrthancPlugins::DatabaseBackendAdapter::Register().
   * 
   * @param context The Orthanc plugin context, as received by OrthancPluginInitialize().
   * @param backend The callbacks of the custom database engine.
   * @param payload Pointer containing private information for the database engine.
   * @param extensions Extensions to the base database SDK that was shipped until Orthanc 0.9.3.
   * @return The context of the database engine (it must not be manually freed).
   * @ingroup Callbacks
   **/
  ORTHANC_PLUGIN_INLINE OrthancPluginDatabaseContext* OrthancPluginRegisterDatabaseBackendV2(
    OrthancPluginContext*                   context,
    const OrthancPluginDatabaseBackend*     backend,
    const OrthancPluginDatabaseExtensions*  extensions,
    void*                                   payload)
  {
    OrthancPluginDatabaseContext* result = NULL;
    _OrthancPluginRegisterDatabaseBackendV2 params;

    if (sizeof(int32_t) != sizeof(_OrthancPluginDatabaseAnswerType))
    {
      return NULL;
    }

    memset(&params, 0, sizeof(params));
    params.backend = backend;
    params.result = &result;
    params.payload = payload;
    params.extensions = extensions;
    params.extensionsSize = sizeof(OrthancPluginDatabaseExtensions);

    if (context->InvokeService(context, _OrthancPluginService_RegisterDatabaseBackendV2, &params) ||
        result == NULL)
    {
      /* Error */
      return NULL;
    }
    else
    {
      return result;
    }
  }


#ifdef  __cplusplus
}
#endif


/** @} */

