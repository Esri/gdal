/*******************************************************************************
 *  Project: NextGIS Web Driver
 *  Purpose: Implements NextGIS Web Driver
 *  Author: Dmitry Baryshnikov, dmitry.baryshnikov@nextgis.com
 *  Language: C++
 *******************************************************************************
 *  The MIT License (MIT)
 *
 *  Copyright (c) 2018-2025, NextGIS <info@nextgis.com>
 *
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "ogr_ngw.h"

// #include <gdalsubdatasetinfo.h>

/*
 * GetHeaders()
 */
static CPLStringList GetHeaders(const std::string &osUserPwdIn = "",
                                const std::string &osConnectTimeout = "",
                                const std::string &osTimeout = "",
                                const std::string &osRetryCount = "",
                                const std::string &osRetryDelay = "")
{
    CPLStringList aosHTTPOptions;
    aosHTTPOptions.AddString("HEADERS=Accept: */*");
    if (!osUserPwdIn.empty())
    {
        aosHTTPOptions.AddString("HTTPAUTH=BASIC");
        std::string osUserPwdOption("USERPWD=");
        osUserPwdOption += osUserPwdIn;
        aosHTTPOptions.AddString(osUserPwdOption.c_str());
    }

    if (!osConnectTimeout.empty())
    {
        aosHTTPOptions.AddNameValue("CONNECTTIMEOUT", osConnectTimeout.c_str());
    }

    if (!osTimeout.empty())
    {
        aosHTTPOptions.AddNameValue("TIMEOUT", osTimeout.c_str());
    }

    if (!osRetryCount.empty())
    {
        aosHTTPOptions.AddNameValue("MAX_RETRY", osRetryCount.c_str());
    }
    if (!osRetryDelay.empty())
    {
        aosHTTPOptions.AddNameValue("RETRY_DELAY", osRetryDelay.c_str());
    }
    return aosHTTPOptions;
}

/*
 * OGRNGWDriverIdentify()
 */

static int OGRNGWDriverIdentify(GDALOpenInfo *poOpenInfo)
{
    return STARTS_WITH_CI(poOpenInfo->pszFilename, "NGW:");
}

/*
 * OGRNGWDriverOpen()
 */

static GDALDataset *OGRNGWDriverOpen(GDALOpenInfo *poOpenInfo)
{
    if (OGRNGWDriverIdentify(poOpenInfo) == 0)
    {
        return nullptr;
    }

    OGRNGWDataset *poDS = new OGRNGWDataset();
    if (!poDS->Open(poOpenInfo->pszFilename, poOpenInfo->papszOpenOptions,
                    poOpenInfo->eAccess == GA_Update, poOpenInfo->nOpenFlags))
    {
        delete poDS;
        poDS = nullptr;
    }

    return poDS;
}

/*
 * OGRNGWDriverCreate()
 *
 * Add new datasource name at the end of URL:
 * NGW:http://some.nextgis.com/resource/0/new_name
 * NGW:http://some.nextgis.com:8000/test/resource/0/new_name
 */

static GDALDataset *
OGRNGWDriverCreate(const char *pszName, CPL_UNUSED int nBands,
                   CPL_UNUSED int nXSize, CPL_UNUSED int nYSize,
                   CPL_UNUSED GDALDataType eDT, char **papszOptions)

{
    NGWAPI::Uri stUri = NGWAPI::ParseUri(pszName);
    CPLErrorReset();
    if (stUri.osPrefix != "NGW")
    {
        CPLError(CE_Failure, CPLE_NotSupported, "Unsupported name %s", pszName);
        return nullptr;
    }

    CPLDebug("NGW", "Parse uri result. URL: %s, ID: %s, New name: %s",
             stUri.osAddress.c_str(), stUri.osResourceId.c_str(),
             stUri.osNewResourceName.c_str());

    std::string osKey = CSLFetchNameValueDef(papszOptions, "KEY", "");
    std::string osDesc = CSLFetchNameValueDef(papszOptions, "DESCRIPTION", "");
    std::string osUserPwd = CSLFetchNameValueDef(
        papszOptions, "USERPWD", CPLGetConfigOption("NGW_USERPWD", ""));

    CPLJSONObject oPayload;
    CPLJSONObject oResource("resource", oPayload);
    oResource.Add("cls", "resource_group");
    oResource.Add("display_name", stUri.osNewResourceName);
    if (!osKey.empty())
    {
        oResource.Add("keyname", osKey);
    }

    if (!osDesc.empty())
    {
        oResource.Add("description", osDesc);
    }

    CPLJSONObject oParent("parent", oResource);
    oParent.Add("id", atoi(stUri.osResourceId.c_str()));

    std::string osConnectTimeout =
        CSLFetchNameValueDef(papszOptions, "CONNECTTIMEOUT",
                             CPLGetConfigOption("NGW_CONNECTTIMEOUT", ""));
    std::string osTimeout = CSLFetchNameValueDef(
        papszOptions, "TIMEOUT", CPLGetConfigOption("NGW_TIMEOUT", ""));

    std::string osNewResourceId = NGWAPI::CreateResource(
        stUri.osAddress, oPayload.Format(CPLJSONObject::PrettyFormat::Plain),
        GetHeaders(osUserPwd, osConnectTimeout, osTimeout));
    if (osNewResourceId == "-1")
    {
        return nullptr;
    }

    OGRNGWDataset *poDS = new OGRNGWDataset();

    if (!poDS->Open(stUri.osAddress, osNewResourceId, papszOptions, true,
                    GDAL_OF_RASTER | GDAL_OF_VECTOR))  // TODO: GDAL_OF_GNM
    {
        delete poDS;
        poDS = nullptr;
    }

    return poDS;
}

/*
 * OGRNGWDriverDelete()
 */
static CPLErr OGRNGWDriverDelete(const char *pszName)
{
    NGWAPI::Uri stUri = NGWAPI::ParseUri(pszName);
    CPLErrorReset();

    if (stUri.osPrefix != "NGW")
    {
        CPLError(CE_Failure, CPLE_NotSupported, "Unsupported name %s", pszName);
        return CE_Failure;
    }

    if (!stUri.osNewResourceName.empty())
    {
        CPLError(CE_Warning, CPLE_NotSupported,
                 "Cannot delete new resource with name %s", pszName);
        return CE_Failure;
    }

    if (stUri.osResourceId == "0")
    {
        CPLError(CE_Failure, CPLE_NotSupported, "Cannot delete resource 0");
        return CE_Failure;
    }

    std::string osUserPwd = CPLGetConfigOption("NGW_USERPWD", "");
    std::string osConnectTimeout = CPLGetConfigOption("NGW_CONNECTTIMEOUT", "");
    std::string osTimeout = CPLGetConfigOption("NGW_TIMEOUT", "");
    std::string osRetryCount = CPLGetConfigOption("NGW_MAX_RETRY", "");
    std::string osRetryDelay = CPLGetConfigOption("NGW_RETRY_DELAY", "");

    auto aosHTTPOptions = GetHeaders(osUserPwd, osConnectTimeout, osTimeout,
                                     osRetryCount, osRetryDelay);

    return NGWAPI::DeleteResource(stUri.osAddress, stUri.osResourceId,
                                  aosHTTPOptions)
               ? CE_None
               : CE_Failure;
}

/*
 * OGRNGWDriverRename()
 */
static CPLErr OGRNGWDriverRename(const char *pszNewName, const char *pszOldName)
{
    NGWAPI::Uri stUri = NGWAPI::ParseUri(pszOldName);
    CPLErrorReset();
    if (stUri.osPrefix != "NGW")
    {
        CPLError(CE_Failure, CPLE_NotSupported, "Unsupported name %s",
                 pszOldName);
        return CE_Failure;
    }
    CPLDebug("NGW", "Parse uri result. URL: %s, ID: %s, New name: %s",
             stUri.osAddress.c_str(), stUri.osResourceId.c_str(), pszNewName);

    std::string osUserPwd = CPLGetConfigOption("NGW_USERPWD", "");
    std::string osConnectTimeout = CPLGetConfigOption("NGW_CONNECTTIMEOUT", "");
    std::string osTimeout = CPLGetConfigOption("NGW_TIMEOUT", "");
    std::string osRetryCount = CPLGetConfigOption("NGW_MAX_RETRY", "");
    std::string osRetryDelay = CPLGetConfigOption("NGW_RETRY_DELAY", "");

    auto aosHTTPOptions = GetHeaders(osUserPwd, osConnectTimeout, osTimeout,
                                     osRetryCount, osRetryDelay);

    return NGWAPI::RenameResource(stUri.osAddress, stUri.osResourceId,
                                  pszNewName, aosHTTPOptions)
               ? CE_None
               : CE_Failure;
}

/*
 * OGRNGWDriverCreateCopy()
 */
static GDALDataset *OGRNGWDriverCreateCopy(const char *pszFilename,
                                           GDALDataset *poSrcDS, int bStrict,
                                           char **papszOptions,
                                           GDALProgressFunc pfnProgress,
                                           void *pProgressData)
{
    // Check destination dataset,
    NGWAPI::Uri stUri = NGWAPI::ParseUri(pszFilename);
    CPLErrorReset();
    if (stUri.osPrefix != "NGW")
    {
        CPLError(CE_Failure, CPLE_NotSupported, "Unsupported name %s",
                 pszFilename);
        return nullptr;
    }

    bool bCloseDS = false;
    std::string osFilename;

    // Check if source GDALDataset is tiff.
    if (EQUAL(poSrcDS->GetDriverName(), "GTiff") == FALSE)
    {
        GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
        // Compress to minimize network transfer.
        const char *apszOptions[] = {"COMPRESS=LZW", "NUM_THREADS=ALL_CPUS",
                                     nullptr};
        std::string osTempFilename = CPLGenerateTempFilenameSafe("ngw_tmp");
        osTempFilename += ".tif";
        GDALDataset *poTmpDS = poDriver->CreateCopy(
            osTempFilename.c_str(), poSrcDS, bStrict,
            const_cast<char **>(apszOptions), pfnProgress, pProgressData);

        if (poTmpDS != nullptr)
        {
            bCloseDS = true;
            osFilename = std::move(osTempFilename);
            poSrcDS = poTmpDS;
        }
        else
        {
            CPLError(CE_Failure, CPLE_NotSupported,
                     "NGW driver doesn't support %s source raster.",
                     poSrcDS->GetDriverName());
            return nullptr;
        }
    }

    if (osFilename.empty())
    {
        // Check if source tiff is local file.
        CPLStringList oaFiles(poSrcDS->GetFileList());
        for (int i = 0; i < oaFiles.size(); ++i)
        {
            // Check extension tif
            const std::string osExt = CPLGetExtensionSafe(oaFiles[i]);
            if (EQUALN(osExt.c_str(), "tif", 3))
            {
                osFilename = oaFiles[i];
                break;
            }
        }
    }

    // Check bands count.
    auto nBands = poSrcDS->GetRasterCount();
    auto nDataType = poSrcDS->GetRasterBand(1)->GetRasterDataType();

    if (bCloseDS)
    {
        delete poSrcDS;
    }

    std::string osKey = CSLFetchNameValueDef(papszOptions, "KEY", "");
    std::string osDesc = CSLFetchNameValueDef(papszOptions, "DESCRIPTION", "");
    std::string osUserPwd = CSLFetchNameValueDef(
        papszOptions, "USERPWD", CPLGetConfigOption("NGW_USERPWD", ""));
    std::string osStyleName =
        CSLFetchNameValueDef(papszOptions, "RASTER_STYLE_NAME", "");

    std::string osConnectTimeout =
        CSLFetchNameValueDef(papszOptions, "CONNECTTIMEOUT",
                             CPLGetConfigOption("NGW_CONNECTTIMEOUT", ""));
    std::string osTimeout = CSLFetchNameValueDef(
        papszOptions, "TIMEOUT", CPLGetConfigOption("NGW_TIMEOUT", ""));
    std::string osRetryCount = CSLFetchNameValueDef(
        papszOptions, "MAX_RETRY", CPLGetConfigOption("NGW_MAX_RETRY", ""));
    std::string osRetryDelay = CSLFetchNameValueDef(
        papszOptions, "RETRY_DELAY", CPLGetConfigOption("NGW_RETRY_DELAY", ""));

    // Send file
    auto aosHTTPOptions = GetHeaders(osUserPwd, osConnectTimeout, osTimeout,
                                     osRetryCount, osRetryDelay);
    CPLJSONObject oFileJson =
        NGWAPI::UploadFile(stUri.osAddress, osFilename, aosHTTPOptions,
                           pfnProgress, pProgressData);

    if (bCloseDS)  // Delete temp tiff file.
    {
        VSIUnlink(osFilename.c_str());
    }

    if (!oFileJson.IsValid())
    {
        return nullptr;
    }

    CPLJSONArray oUploadMeta = oFileJson.GetArray("upload_meta");
    if (!oUploadMeta.IsValid() || oUploadMeta.Size() == 0)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Get unexpected response: %s.",
                 oFileJson.Format(CPLJSONObject::PrettyFormat::Plain).c_str());
        return nullptr;
    }

    // Create raster layer
    // Create payload
    CPLJSONObject oPayloadRaster;
    CPLJSONObject oResource("resource", oPayloadRaster);
    oResource.Add("cls", "raster_layer");
    oResource.Add("display_name", stUri.osNewResourceName);
    if (!osKey.empty())
    {
        oResource.Add("keyname", osKey);
    }

    if (!osDesc.empty())
    {
        oResource.Add("description", osDesc);
    }

    CPLJSONObject oParent("parent", oResource);
    oParent.Add("id", atoi(stUri.osResourceId.c_str()));

    CPLJSONObject oRasterLayer("raster_layer", oPayloadRaster);
    oRasterLayer.Add("source", oUploadMeta[0]);

    CPLJSONObject oSrs("srs", oRasterLayer);
    oSrs.Add("id", 3857);  // Now only Web Mercator supported.

    auto osRasterResourceId = NGWAPI::CreateResource(
        stUri.osAddress,
        oPayloadRaster.Format(CPLJSONObject::PrettyFormat::Plain),
        aosHTTPOptions);
    if (osRasterResourceId == "-1")
    {
        return nullptr;
    }

    // Create raster style
    CPLJSONObject oPayloadRasterStyle;
    CPLJSONObject oResourceStyle("resource", oPayloadRasterStyle);

    // NGW v3.1 supported different raster types: 1 band and 16/32 bit, RGB/RGBA
    // rasters and etc.
    // For RGB/RGBA rasters we can create default raster_style.
    // For other types - qml style file path is mandatory.
    std::string osQMLPath =
        CSLFetchNameValueDef(papszOptions, "RASTER_QML_PATH", "");

    bool bCreateStyle = true;
    if (osQMLPath.empty())
    {
        if ((nBands == 3 || nBands == 4) && nDataType == GDT_Byte)
        {
            oResourceStyle.Add("cls", "raster_style");
        }
        else
        {
            CPLError(CE_Warning, CPLE_NotSupported,
                     "Default NGW raster style supports only 3 (RGB) or 4 "
                     "(RGBA) and 8 "
                     "bit byte bands. Raster has %d bands and data type %s",
                     nBands, GDALGetDataTypeName(nDataType));
            bCreateStyle = false;
        }
    }
    else
    {
        oResourceStyle.Add("cls", "qgis_raster_style");

        // Upload QML file
        oFileJson =
            NGWAPI::UploadFile(stUri.osAddress, osQMLPath, aosHTTPOptions,
                               pfnProgress, pProgressData);
        oUploadMeta = oFileJson.GetArray("upload_meta");
        if (!oUploadMeta.IsValid() || oUploadMeta.Size() == 0)
        {
            CPLError(
                CE_Failure, CPLE_AppDefined, "Get unexpected response: %s.",
                oFileJson.Format(CPLJSONObject::PrettyFormat::Plain).c_str());
            return nullptr;
        }
        CPLJSONObject oQGISRasterStyle("qgis_raster_style",
                                       oPayloadRasterStyle);
        oQGISRasterStyle.Add("file_upload", oUploadMeta[0]);
    }

    if (bCreateStyle)
    {
        if (osStyleName.empty())
        {
            osStyleName = stUri.osNewResourceName;
        }
        oResourceStyle.Add("display_name", osStyleName);
        CPLJSONObject oParentRaster("parent", oResourceStyle);
        oParentRaster.Add("id", atoi(osRasterResourceId.c_str()));

        auto osStyleResourceId = NGWAPI::CreateResource(
            stUri.osAddress,
            oPayloadRasterStyle.Format(CPLJSONObject::PrettyFormat::Plain),
            aosHTTPOptions);
        if (osStyleResourceId == "-1")
        {
            return nullptr;
        }
    }

    OGRNGWDataset *poDS = new OGRNGWDataset();

    if (!poDS->Open(stUri.osAddress, osRasterResourceId, papszOptions, true,
                    GDAL_OF_RASTER))
    {
        delete poDS;
        poDS = nullptr;
    }

    return poDS;
}

/*
 * RegisterOGRNGW()
 */

void RegisterOGRNGW()
{
    if (GDALGetDriverByName("NGW") != nullptr)
    {
        return;
    }

    GDALDriver *poDriver = new GDALDriver();

    poDriver->SetDescription("NGW");
    poDriver->SetMetadataItem(GDAL_DMD_LONGNAME, "NextGIS Web");
    poDriver->SetMetadataItem(GDAL_DCAP_RASTER, "YES");
    poDriver->SetMetadataItem(GDAL_DCAP_VECTOR, "YES");
    poDriver->SetMetadataItem(GDAL_DCAP_CREATE_LAYER, "YES");
    poDriver->SetMetadataItem(GDAL_DCAP_DELETE_LAYER, "YES");
    poDriver->SetMetadataItem(GDAL_DCAP_CREATE_FIELD, "YES");
    poDriver->SetMetadataItem(GDAL_DCAP_DELETE_FIELD, "YES");
    poDriver->SetMetadataItem(GDAL_DMD_SUBDATASETS, "YES");
    poDriver->SetMetadataItem(GDAL_DMD_HELPTOPIC, "drivers/vector/ngw.html");
    poDriver->SetMetadataItem(GDAL_DMD_CONNECTION_PREFIX, "NGW:");
    poDriver->SetMetadataItem(GDAL_DCAP_Z_GEOMETRIES, "YES");
    poDriver->SetMetadataItem(GDAL_DMD_SUPPORTED_SQL_DIALECTS,
                              "NATIVE OGRSQL SQLITE");

    poDriver->SetMetadataItem(GDAL_DMD_CREATIONDATATYPES, "Byte");
    poDriver->SetMetadataItem(GDAL_DMD_ALTER_FIELD_DEFN_FLAGS,
                              "Name AlternativeName Domain");
    poDriver->SetMetadataItem(GDAL_DMD_CREATION_FIELD_DEFN_FLAGS,
                              "AlternativeName Domain");
    poDriver->SetMetadataItem(GDAL_DCAP_CREATECOPY, "YES");
    poDriver->SetMetadataItem(GDAL_DCAP_FIELD_DOMAINS, "YES");

    poDriver->SetMetadataItem(
        GDAL_DMD_OPENOPTIONLIST,
        "<OpenOptionList>"
        "   <Option name='USERPWD' scope='raster,vector' type='string' "
        "description='Username and password, separated by colon'/>"
        "   <Option name='PAGE_SIZE' scope='vector' type='integer' "
        "description='Limit feature count while fetching from server. Default "
        "value is -1 - no limit' default='-1'/>"
        "   <Option name='BATCH_SIZE' scope='vector' type='integer' "
        "description='Size of feature insert and update operations cache "
        "before send to server. If batch size is -1 batch mode is disabled' "
        "default='-1'/>"
        "   <Option name='NATIVE_DATA' scope='vector' type='boolean' "
        "description='Whether to store the native Json representation of "
        "extensions key. If EXTENSIONS not set or empty, NATIVE_DATA defaults "
        "to NO' default='NO'/>"
        "   <Option name='CACHE_EXPIRES' scope='raster' type='integer' "
        "description='Time in seconds cached files will stay valid. If cached "
        "file expires it is deleted when maximum size of cache is reached. "
        "Also expired file can be overwritten by the new one from web' "
        "default='604800'/>"
        "   <Option name='CACHE_MAX_SIZE' scope='raster' type='integer' "
        "description='The cache maximum size in bytes. If cache reached "
        "maximum size, expired cached files will be deleted' "
        "default='67108864'/>"
        "   <Option name='JSON_DEPTH' scope='raster,vector' type='integer' "
        "description='The depth of json response that can be parsed. If depth "
        "is greater than this value, parse error occurs' default='32'/>"
        "   <Option name='EXTENSIONS' scope='vector' type='string' "
        "description='Comma separated extensions list. Available are "
        "description and attachment' default=''/>"
        "   <Option name='CONNECTTIMEOUT' scope='raster,vector' type='integer' "
        "description='Maximum delay for the connection to be established "
        "before "
        "being aborted in seconds'/>"
        "   <Option name='TIMEOUT' scope='raster,vector' type='integer' "
        "description='Maximum delay for the whole request to complete before "
        "being aborted in seconds'/>"
        "   <Option name='MAX_RETRY' scope='raster,vector' type='integer' "
        "description='Maximum number of retry attempts if a 429, 502, 503 or "
        "504 "
        "HTTP error occurs'/>"
        "   <Option name='RETRY_DELAY' scope='raster,vector' type='integer' "
        "description='Number of seconds between retry attempts'/>"
        "</OpenOptionList>");

    poDriver->SetMetadataItem(
        GDAL_DMD_CREATIONOPTIONLIST,
        "<CreationOptionList>"
        "   <Option name='KEY' scope='raster,vector' type='string' "
        "description='Key value. Must be unique in whole NextGIS Web "
        "instance'/>"
        "   <Option name='DESCRIPTION' scope='raster,vector' type='string' "
        "description='Resource description'/>"
        "   <Option name='RASTER_STYLE_NAME' scope='raster' type='string' "
        "description='Raster layer style name'/>"
        "   <Option name='USERPWD' scope='raster,vector' type='string' "
        "description='Username and password, separated by colon'/>"
        "   <Option name='PAGE_SIZE' scope='vector' type='integer' "
        "description='Limit feature count while fetching from server. Default "
        "value is -1 - no limit' default='-1'/>"
        "   <Option name='BATCH_SIZE' scope='vector' type='integer' "
        "description='Size of feature insert and update operations cache "
        "before send to server. If batch size is -1 batch mode is disabled' "
        "default='-1'/>"
        "   <Option name='NATIVE_DATA' scope='vector' type='boolean' "
        "description='Whether to store the native Json representation of "
        "extensions key. If EXTENSIONS not set or empty, NATIVE_DATA defaults "
        "to NO' default='NO'/>"
        "   <Option name='CACHE_EXPIRES' scope='raster' type='integer' "
        "description='Time in seconds cached files will stay valid. If cached "
        "file expires it is deleted when maximum size of cache is reached. "
        "Also expired file can be overwritten by the new one from web' "
        "default='604800'/>"
        "   <Option name='CACHE_MAX_SIZE' scope='raster' type='integer' "
        "description='The cache maximum size in bytes. If cache reached "
        "maximum size, expired cached files will be deleted' "
        "default='67108864'/>"
        "   <Option name='JSON_DEPTH' scope='raster,vector' type='integer' "
        "description='The depth of json response that can be parsed. If depth "
        "is greater than this value, parse error occurs' default='32'/>"
        "   <Option name='RASTER_QML_PATH' scope='raster' type='string' "
        "description='Raster QMS style path'/>"
        "   <Option name='EXTENSIONS' scope='vector' type='string' "
        "description='Comma separated extensions list. Available are "
        "description and attachment' default=''/>"
        "   <Option name='CONNECTTIMEOUT' scope='raster,vector' type='integer' "
        "description='Maximum delay for the connection to be established "
        "before "
        "being aborted in seconds'/>"
        "   <Option name='TIMEOUT' scope='raster,vector' type='integer' "
        "description='Maximum delay for the whole request to complete before "
        "being aborted in seconds'/>"
        "   <Option name='MAX_RETRY' scope='raster,vector' type='integer' "
        "description='Maximum number of retry attempts if a 429, 502, 503 or "
        "504 "
        "HTTP error occurs'/>"
        "   <Option name='RETRY_DELAY' scope='raster,vector' type='integer' "
        "description='Number of seconds between retry attempts'/>"
        "</CreationOptionList>");

    poDriver->SetMetadataItem(
        GDAL_DS_LAYER_CREATIONOPTIONLIST,
        "<LayerCreationOptionList>"
        "   <Option name='OVERWRITE' type='boolean' description='Whether to "
        "overwrite an existing table with the layer name to be created' "
        "default='NO'/>"
        "   <Option name='KEY' type='string' description='Key value. Must be "
        "unique in whole NextGIS Web instance'/>"
        "   <Option name='DESCRIPTION' type='string' description='Resource "
        "description'/>"
        "</LayerCreationOptionList>");

    poDriver->SetMetadataItem(
        GDAL_DMD_CREATIONFIELDDATATYPES,
        "Integer Integer64 Real String Date DateTime Time");
    poDriver->SetMetadataItem(GDAL_DMD_CREATION_FIELD_DEFN_FLAGS,
                              "AlternativeName");
    poDriver->SetMetadataItem(GDAL_DCAP_NOTNULL_GEOMFIELDS, "YES");
    poDriver->SetMetadataItem(GDAL_DCAP_MULTIPLE_VECTOR_LAYERS, "YES");
    poDriver->SetMetadataItem(GDAL_DCAP_RENAME_LAYERS, "YES");
    poDriver->SetMetadataItem(GDAL_DMD_CREATION_FIELD_DOMAIN_TYPES, "Coded");

    poDriver->pfnOpen = OGRNGWDriverOpen;
    poDriver->pfnIdentify = OGRNGWDriverIdentify;
    poDriver->pfnCreate = OGRNGWDriverCreate;
    poDriver->pfnCreateCopy = OGRNGWDriverCreateCopy;
    poDriver->pfnDelete = OGRNGWDriverDelete;
    poDriver->pfnRename = OGRNGWDriverRename;

    GetGDALDriverManager()->RegisterDriver(poDriver);
}
