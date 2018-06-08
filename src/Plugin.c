#include <orthanc/OrthancCPlugin.h>

#include <string.h>
#include <stdio.h>

static OrthancPluginContext* context = NULL;

static OrthancPluginErrorCode customError;


ORTHANC_PLUGINS_API OrthancPluginErrorCode Callback1(OrthancPluginRestOutput* output,
                                                     const char* url,
                                                     const OrthancPluginHttpRequest* request)
{
  char buffer[1024];
  uint32_t i;

  sprintf(buffer, "Callback on URL [%s] with body [%s]\n", url, request->body);
  OrthancPluginLogWarning(context, buffer);

  OrthancPluginSetCookie(context, output, "hello", "world");
  OrthancPluginAnswerBuffer(context, output, buffer, strlen(buffer), "text/plain");

  OrthancPluginLogWarning(context, "");    

  for (i = 0; i < request->groupsCount; i++)
  {
    sprintf(buffer, "  REGEX GROUP %d = [%s]", i, request->groups[i]);
    OrthancPluginLogWarning(context, buffer);    
  }

  OrthancPluginLogWarning(context, "");    

  for (i = 0; i < request->getCount; i++)
  {
    sprintf(buffer, "  GET [%s] = [%s]", request->getKeys[i], request->getValues[i]);
    OrthancPluginLogWarning(context, buffer);    
  }

  OrthancPluginLogWarning(context, "");

  for (i = 0; i < request->headersCount; i++)
  {
    sprintf(buffer, "  HEADERS [%s] = [%s]", request->headersKeys[i], request->headersValues[i]);
    OrthancPluginLogWarning(context, buffer);    
  }

  OrthancPluginLogWarning(context, "");

  return OrthancPluginErrorCode_Success;
}


ORTHANC_PLUGINS_API OrthancPluginErrorCode Callback2(OrthancPluginRestOutput* output,
                                                     const char* url,
                                                     const OrthancPluginHttpRequest* request)
{
  /* Answer with a sample 16bpp image. */

  uint16_t buffer[256 * 256];
  uint32_t x, y, value;

  if (request->method != OrthancPluginHttpMethod_Get)
  {
    OrthancPluginSendMethodNotAllowed(context, output, "GET");
  }
  else
  {
    value = 0;
    for (y = 0; y < 256; y++)
    {
      for (x = 0; x < 256; x++, value++)
      {
        buffer[value] = value;
      }
    }

    OrthancPluginCompressAndAnswerPngImage(context, output, OrthancPluginPixelFormat_Grayscale16,
                                           256, 256, sizeof(uint16_t) * 256, buffer);
  }

  return OrthancPluginErrorCode_Success;
}


ORTHANC_PLUGINS_API OrthancPluginErrorCode Callback3(OrthancPluginRestOutput* output,
                                                     const char* url,
                                                     const OrthancPluginHttpRequest* request)
{
  if (request->method != OrthancPluginHttpMethod_Get)
  {
    OrthancPluginSendMethodNotAllowed(context, output, "GET");
  }
  else
  {
    OrthancPluginMemoryBuffer dicom;
    if (!OrthancPluginGetDicomForInstance(context, &dicom, request->groups[0]))
    {
      /* No error, forward the DICOM file */
      OrthancPluginAnswerBuffer(context, output, dicom.data, dicom.size, "application/dicom");

      /* Free memory */
      OrthancPluginFreeMemoryBuffer(context, &dicom);
    }
  }

  return OrthancPluginErrorCode_Success;
}


ORTHANC_PLUGINS_API OrthancPluginErrorCode Callback4(OrthancPluginRestOutput* output,
                                                     const char* url,
                                                     const OrthancPluginHttpRequest* request)
{
  /* Answer with a sample 8bpp image. */

  uint8_t  buffer[256 * 256];
  uint32_t x, y, value;

  if (request->method != OrthancPluginHttpMethod_Get)
  {
    OrthancPluginSendMethodNotAllowed(context, output, "GET");
  }
  else
  {
    value = 0;
    for (y = 0; y < 256; y++)
    {
      for (x = 0; x < 256; x++, value++)
      {
        buffer[value] = x;
      }
    }

    OrthancPluginCompressAndAnswerPngImage(context, output, OrthancPluginPixelFormat_Grayscale8,
                                           256, 256, 256, buffer);
  }

  return OrthancPluginErrorCode_Success;
}


ORTHANC_PLUGINS_API OrthancPluginErrorCode Callback5(OrthancPluginRestOutput* output,
                                                     const char* url,
                                                     const OrthancPluginHttpRequest* request)
{
  /**
   * Demonstration the difference between the
   * "OrthancPluginRestApiXXX()" and the
   * "OrthancPluginRestApiXXXAfterPlugins()" mechanisms to forward
   * REST calls.
   *
   * # curl http://localhost:8042/forward/built-in/system
   * # curl http://localhost:8042/forward/plugins/system
   * # curl http://localhost:8042/forward/built-in/plugin/image
   *   => FAILURE (because the "/plugin/image" URI is implemented by this plugin)
   * # curl http://localhost:8042/forward/plugins/plugin/image  => SUCCESS
   **/

  OrthancPluginMemoryBuffer tmp;
  int isBuiltIn, error;

  if (request->method != OrthancPluginHttpMethod_Get)
  {
    OrthancPluginSendMethodNotAllowed(context, output, "GET");
    return 0;
  }

  isBuiltIn = strcmp("plugins", request->groups[0]);
 
  if (isBuiltIn)
  {
    error = OrthancPluginRestApiGet(context, &tmp, request->groups[1]);
  }
  else
  {
    error = OrthancPluginRestApiGetAfterPlugins(context, &tmp, request->groups[1]);
  }

  if (error)
  {
    return OrthancPluginErrorCode_InternalError;
  }
  else
  {
    OrthancPluginAnswerBuffer(context, output, tmp.data, tmp.size, "application/octet-stream");
    OrthancPluginFreeMemoryBuffer(context, &tmp);
    return OrthancPluginErrorCode_Success;
  }
}


ORTHANC_PLUGINS_API OrthancPluginErrorCode CallbackCreateDicom(OrthancPluginRestOutput* output,
                                                               const char* url,
                                                               const OrthancPluginHttpRequest* request)
{
  const char* pathLocator = "\"Path\" : \"";
  char info[1024];
  char *id, *eos;
  OrthancPluginMemoryBuffer tmp;

  if (request->method != OrthancPluginHttpMethod_Post)
  {
    OrthancPluginSendMethodNotAllowed(context, output, "POST");
  }
  else
  {
    /* Make POST request to create a new DICOM instance */
    sprintf(info, "{\"PatientName\":\"Test\"}");
    OrthancPluginRestApiPost(context, &tmp, "/tools/create-dicom", info, strlen(info));

    /**
     * Recover the ID of the created instance is constructed by a
     * quick-and-dirty parsing of a JSON string.
     **/
    id = strstr((char*) tmp.data, pathLocator) + strlen(pathLocator);
    eos = strchr(id, '\"');
    eos[0] = '\0';

    /* Delete the newly created DICOM instance. */
    OrthancPluginRestApiDelete(context, id);
    OrthancPluginFreeMemoryBuffer(context, &tmp);

    /* Set some cookie */
    OrthancPluginSetCookie(context, output, "hello", "world");

    /* Set some HTTP header */
    OrthancPluginSetHttpHeader(context, output, "Cache-Control", "max-age=0, no-cache");
    
    OrthancPluginAnswerBuffer(context, output, "OK\n", 3, "text/plain");
  }

  return OrthancPluginErrorCode_Success;
}


ORTHANC_PLUGINS_API OrthancPluginErrorCode OnStoredCallback(OrthancPluginDicomInstance* instance,
                                                            const char* instanceId)
{
  char buffer[256];
  FILE* fp;
  char* json;
  static int first = 1;

  sprintf(buffer, "Just received a DICOM instance of size %d and ID %s from origin %d (AET %s)", 
          (int) OrthancPluginGetInstanceSize(context, instance), instanceId, 
          OrthancPluginGetInstanceOrigin(context, instance),
          OrthancPluginGetInstanceRemoteAet(context, instance));

  OrthancPluginLogWarning(context, buffer);  

  fp = fopen("PluginReceivedInstance.dcm", "wb");
  fwrite(OrthancPluginGetInstanceData(context, instance),
         OrthancPluginGetInstanceSize(context, instance), 1, fp);
  fclose(fp);

  json = OrthancPluginGetInstanceSimplifiedJson(context, instance);
  if (first)
  {
    /* Only print the first DICOM instance */
    printf("[%s]\n", json);
    first = 0;
  }
  OrthancPluginFreeString(context, json);

  if (OrthancPluginHasInstanceMetadata(context, instance, "ReceptionDate"))
  {
    printf("Received on [%s]\n", OrthancPluginGetInstanceMetadata(context, instance, "ReceptionDate"));
  }
  else
  {
    OrthancPluginLogError(context, "Instance has no reception date, should never happen!");
  }

  return OrthancPluginErrorCode_Success;
}


ORTHANC_PLUGINS_API OrthancPluginErrorCode OnChangeCallback(OrthancPluginChangeType changeType,
                                                            OrthancPluginResourceType resourceType,
                                                            const char* resourceId)
{
  char info[1024];
  OrthancPluginMemoryBuffer tmp;

  sprintf(info, "Change %d on resource %s of type %d", changeType, resourceId, resourceType);
  OrthancPluginLogWarning(context, info);

  if (changeType == OrthancPluginChangeType_NewInstance)
  {
    sprintf(info, "/instances/%s/metadata/AnonymizedFrom", resourceId);
    if (OrthancPluginRestApiGet(context, &tmp, info) == 0)
    {
      sprintf(info, "  Instance %s comes from the anonymization of instance", resourceId);
      strncat(info, (const char*) tmp.data, tmp.size);
      OrthancPluginLogWarning(context, info);
      OrthancPluginFreeMemoryBuffer(context, &tmp);
    }
  }

  return OrthancPluginErrorCode_Success;
}


ORTHANC_PLUGINS_API int32_t FilterIncomingHttpRequest(OrthancPluginHttpMethod  method,
                                                      const char*              uri,
                                                      const char*              ip,
                                                      uint32_t                 headersCount,
                                                      const char* const*       headersKeys,
                                                      const char* const*       headersValues)
{
  uint32_t i;

  if (headersCount > 0)
  {
    OrthancPluginLogInfo(context, "HTTP headers of an incoming REST request:");
    for (i = 0; i < headersCount; i++)
    {
      char info[1024];
      sprintf(info, "  %s: %s", headersKeys[i], headersValues[i]);
      OrthancPluginLogInfo(context, info);
    }
  }

  if (method == OrthancPluginHttpMethod_Get ||
      method == OrthancPluginHttpMethod_Post)
  {
    return 1;  /* Allowed */
  }
  else
  {
    return 0;  /* Only allow GET and POST requests */
  }
}


ORTHANC_PLUGINS_API int32_t OrthancPluginInitialize(OrthancPluginContext* c)
{
  OrthancPluginMemoryBuffer tmp;
  char info[1024], *s;
  int counter, i;
  OrthancPluginDictionaryEntry entry;

  context = c;
  OrthancPluginLogWarning(context, "Sample plugin is initializing");

  /* Check the version of the Orthanc core */
  if (OrthancPluginCheckVersion(c) == 0)
  {
    sprintf(info, "Your version of Orthanc (%s) must be above %d.%d.%d to run this plugin",
            c->orthancVersion,
            ORTHANC_PLUGINS_MINIMAL_MAJOR_NUMBER,
            ORTHANC_PLUGINS_MINIMAL_MINOR_NUMBER,
            ORTHANC_PLUGINS_MINIMAL_REVISION_NUMBER);
    OrthancPluginLogError(context, info);
    return -1;
  }

  /* Print some information about Orthanc */
  sprintf(info, "The version of Orthanc is '%s'", context->orthancVersion);
  OrthancPluginLogWarning(context, info);

  s = OrthancPluginGetOrthancPath(context);
  sprintf(info, "  Path to Orthanc: %s", s);
  OrthancPluginLogWarning(context, info);
  OrthancPluginFreeString(context, s);

  s = OrthancPluginGetOrthancDirectory(context);
  sprintf(info, "  Directory of Orthanc: %s", s);
  OrthancPluginLogWarning(context, info);
  OrthancPluginFreeString(context, s);

  s = OrthancPluginGetConfiguration(context);
  sprintf(info, "  Content of the configuration file:\n");
  OrthancPluginLogWarning(context, info);
  OrthancPluginLogWarning(context, s);
  OrthancPluginFreeString(context, s);

  /* Print the command-line arguments of Orthanc */
  counter = OrthancPluginGetCommandLineArgumentsCount(context);
  for (i = 0; i < counter; i++)
  {
    s = OrthancPluginGetCommandLineArgument(context, i);
    sprintf(info, "  Command-line argument %d: \"%s\"", i, s);
    OrthancPluginLogWarning(context, info);
    OrthancPluginFreeString(context, s);    
  }

  /* Register the callbacks */
  OrthancPluginRegisterRestCallback(context, "/(plu.*)/hello", Callback1);
  OrthancPluginRegisterRestCallback(context, "/plu.*/image", Callback2);
  OrthancPluginRegisterRestCallback(context, "/plugin/instances/([^/]+)/info", Callback3);
  OrthancPluginRegisterRestCallback(context, "/instances/([^/]+)/preview", Callback4);
  OrthancPluginRegisterRestCallback(context, "/forward/(built-in)(/.+)", Callback5);
  OrthancPluginRegisterRestCallback(context, "/forward/(plugins)(/.+)", Callback5);
  OrthancPluginRegisterRestCallback(context, "/plugin/create", CallbackCreateDicom);

  OrthancPluginRegisterOnStoredInstanceCallback(context, OnStoredCallback);
  OrthancPluginRegisterOnChangeCallback(context, OnChangeCallback);
  OrthancPluginRegisterIncomingHttpRequestFilter(context, FilterIncomingHttpRequest);

  /* Declare several properties of the plugin */
  OrthancPluginSetRootUri(context, "/plugin/hello");
  OrthancPluginSetDescription(context, "This is the description of the sample plugin that can be seen in Orthanc Explorer.");
  OrthancPluginExtendOrthancExplorer(context, "alert('Hello Orthanc! From sample plugin with love.');");

  /* Make REST requests to the built-in Orthanc API */
  memset(&tmp, 0, sizeof(tmp));
  OrthancPluginRestApiGet(context, &tmp, "/changes");
  OrthancPluginFreeMemoryBuffer(context, &tmp);
  OrthancPluginRestApiGet(context, &tmp, "/changes?limit=1");
  OrthancPluginFreeMemoryBuffer(context, &tmp);
  
  /* Play with PUT by defining a new target modality. */
  sprintf(info, "[ \"STORESCP\", \"localhost\", 2000 ]");
  OrthancPluginRestApiPut(context, &tmp, "/modalities/demo", info, strlen(info));

  customError = OrthancPluginRegisterErrorCode(context, 4, 402, "Hello world");

  OrthancPluginRegisterDictionaryTag(context, 0x0014, 0x1020, OrthancPluginValueRepresentation_DA,
                                     "ValidationExpiryDate", 1, 1);

  OrthancPluginLookupDictionary(context, &entry, "ValidationExpiryDate");
  OrthancPluginLookupDictionary(context, &entry, "0010-0010");

  return 0;
}


ORTHANC_PLUGINS_API void OrthancPluginFinalize()
{
  OrthancPluginLogWarning(context, "Sample plugin is finalizing");
}


ORTHANC_PLUGINS_API const char* OrthancPluginGetName()
{
  return "sample";
}


ORTHANC_PLUGINS_API const char* OrthancPluginGetVersion()
{
  return "1.0";
}

