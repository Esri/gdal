/******************************************************************************
 *
 * Project:  GDAL
 * Purpose:  GDALGeorefPamDataset with helper to read georeferencing and other
 *           metadata from JP2Boxes
 * Author:   Even Rouault <even dot rouault at spatialys.com>
 *
 ******************************************************************************
 * Copyright (c) 2002, Frank Warmerdam <warmerdam@pobox.com>
 * Copyright (c) 2013, Even Rouault <even dot rouault at spatialys.com>
 *
 * SPDX-License-Identifier: MIT
 ****************************************************************************/

#include "cpl_port.h"
#include "gdaljp2abstractdataset.h"

#include <cstring>

#include <string>

#include "cpl_conv.h"
#include "cpl_error.h"
#include "cpl_minixml.h"
#include "cpl_string.h"
#include "cpl_vsi.h"
#include "gdal.h"
#include "gdal_mdreader.h"
#include "gdal_priv.h"
#include "gdaljp2metadata.h"
#include "ogrsf_frmts.h"
#include "memdataset.h"

/*! @cond Doxygen_Suppress */

/************************************************************************/
/*                     GDALJP2AbstractDataset()                         */
/************************************************************************/

GDALJP2AbstractDataset::GDALJP2AbstractDataset() = default;

/************************************************************************/
/*                     ~GDALJP2AbstractDataset()                        */
/************************************************************************/

GDALJP2AbstractDataset::~GDALJP2AbstractDataset()
{
    CPLFree(pszWldFilename);
    GDALJP2AbstractDataset::CloseDependentDatasets();
    CSLDestroy(papszMetadataFiles);
}

/************************************************************************/
/*                      CloseDependentDatasets()                        */
/************************************************************************/

int GDALJP2AbstractDataset::CloseDependentDatasets()
{
    const bool bRet =
        CPL_TO_BOOL(GDALGeorefPamDataset::CloseDependentDatasets());
    if (poMemDS == nullptr)
        return bRet;

    GDALClose(poMemDS);
    poMemDS = nullptr;
    return true;
}

/************************************************************************/
/*                          LoadJP2Metadata()                           */
/************************************************************************/

void GDALJP2AbstractDataset::LoadJP2Metadata(GDALOpenInfo *poOpenInfo,
                                             const char *pszOverrideFilenameIn,
                                             VSILFILE *fpBox)
{
    const char *pszOverrideFilename = pszOverrideFilenameIn;
    if (pszOverrideFilename == nullptr)
        pszOverrideFilename = poOpenInfo->pszFilename;

    /* -------------------------------------------------------------------- */
    /*      Identify authorized georeferencing sources                      */
    /* -------------------------------------------------------------------- */
    const char *pszGeorefSourcesOption =
        CSLFetchNameValue(poOpenInfo->papszOpenOptions, "GEOREF_SOURCES");
    bool bGeorefSourcesConfigOption = pszGeorefSourcesOption != nullptr;
    CPLString osGeorefSources =
        (pszGeorefSourcesOption) ? pszGeorefSourcesOption
                                 : CPLGetConfigOption("GDAL_GEOREF_SOURCES",
                                                      "PAM,INTERNAL,WORLDFILE");
    size_t nInternalIdx = osGeorefSources.ifind("INTERNAL");
    if (nInternalIdx != std::string::npos &&
        (nInternalIdx == 0 || osGeorefSources[nInternalIdx - 1] == ',') &&
        (nInternalIdx + strlen("INTERNAL") == osGeorefSources.size() ||
         osGeorefSources[nInternalIdx + strlen("INTERNAL")] == ','))
    {
        osGeorefSources.replace(nInternalIdx, strlen("INTERNAL"),
                                "GEOJP2,GMLJP2,MSIG");
    }
    const CPLStringList aosTokens(CSLTokenizeString2(osGeorefSources, ",", 0));
    m_bGotPAMGeorefSrcIndex = true;
    m_nPAMGeorefSrcIndex = aosTokens.FindString("PAM");
    const int nGEOJP2Index = aosTokens.FindString("GEOJP2");
    const int nGMLJP2Index = aosTokens.FindString("GMLJP2");
    const int nMSIGIndex = aosTokens.FindString("MSIG");
    m_nWORLDFILEIndex = aosTokens.FindString("WORLDFILE");

    if (bGeorefSourcesConfigOption)
    {
        for (const char *pszToken : aosTokens)
        {
            if (!EQUAL(pszToken, "PAM") && !EQUAL(pszToken, "GEOJP2") &&
                !EQUAL(pszToken, "GMLJP2") && !EQUAL(pszToken, "MSIG") &&
                !EQUAL(pszToken, "WORLDFILE") && !EQUAL(pszToken, "NONE"))
            {
                CPLError(CE_Warning, CPLE_AppDefined,
                         "Unhandled value %s in GEOREF_SOURCES", pszToken);
            }
        }
    }

    /* -------------------------------------------------------------------- */
    /*      Check for georeferencing information.                           */
    /* -------------------------------------------------------------------- */
    GDALJP2Metadata oJP2Geo;
    int nIndexUsed = -1;
    if ((((fpBox != nullptr || poOpenInfo->fpL != nullptr) &&
          pszOverrideFilenameIn == nullptr &&
          oJP2Geo.ReadAndParse(fpBox ? fpBox : poOpenInfo->fpL, nGEOJP2Index,
                               nGMLJP2Index, nMSIGIndex, &nIndexUsed)) ||
         (!((fpBox != nullptr || poOpenInfo->fpL != nullptr) &&
            pszOverrideFilenameIn == nullptr) &&
          oJP2Geo.ReadAndParse(pszOverrideFilename, nGEOJP2Index, nGMLJP2Index,
                               nMSIGIndex, m_nWORLDFILEIndex, &nIndexUsed))) &&
        (nGMLJP2Index >= 0 || nGEOJP2Index >= 0 || nMSIGIndex >= 0 ||
         m_nWORLDFILEIndex >= 0))
    {
        m_oSRS = oJP2Geo.m_oSRS;
        if (!m_oSRS.IsEmpty())
            m_nProjectionGeorefSrcIndex = nIndexUsed;
        bGeoTransformValid = oJP2Geo.m_bHaveGeoTransform;
        if (bGeoTransformValid)
            m_nGeoTransformGeorefSrcIndex = nIndexUsed;
        m_gt = oJP2Geo.m_gt;
        nGCPCount = oJP2Geo.nGCPCount;
        if (nGCPCount)
            m_nGCPGeorefSrcIndex = nIndexUsed;
        pasGCPList = GDALDuplicateGCPs(oJP2Geo.nGCPCount, oJP2Geo.pasGCPList);

        if (oJP2Geo.bPixelIsPoint)
        {
            m_bPixelIsPoint = true;
            m_nPixelIsPointGeorefSrcIndex = nIndexUsed;
        }
        if (oJP2Geo.papszRPCMD)
        {
            m_papszRPC = CSLDuplicate(oJP2Geo.papszRPCMD);
            m_nRPCGeorefSrcIndex = nIndexUsed;
        }
    }

    /* -------------------------------------------------------------------- */
    /*      Report XML UUID box in a dedicated metadata domain              */
    /* -------------------------------------------------------------------- */
    if (oJP2Geo.pszXMPMetadata)
    {
        char *apszMDList[2] = {oJP2Geo.pszXMPMetadata, nullptr};
        GDALDataset::SetMetadata(apszMDList, "xml:XMP");
    }

    /* -------------------------------------------------------------------- */
    /*      Do we have any XML boxes we would like to treat as special      */
    /*      domain metadata? (Note: the GDAL multidomain metadata XML box   */
    /*      has been excluded and is dealt a few lines below.               */
    /* -------------------------------------------------------------------- */

    for (int iBox = 0;
         oJP2Geo.papszGMLMetadata && oJP2Geo.papszGMLMetadata[iBox] != nullptr;
         ++iBox)
    {
        char *pszName = nullptr;
        const char *pszXML =
            CPLParseNameValue(oJP2Geo.papszGMLMetadata[iBox], &pszName);
        CPLString osDomain;
        osDomain.Printf("xml:%s", pszName);
        char *apszMDList[2] = {const_cast<char *>(pszXML), nullptr};

        GDALDataset::SetMetadata(apszMDList, osDomain);

        CPLFree(pszName);
    }

    /* -------------------------------------------------------------------- */
    /*      Do we have GDAL metadata?                                       */
    /* -------------------------------------------------------------------- */
    if (oJP2Geo.pszGDALMultiDomainMetadata != nullptr)
    {
        CPLErr eLastErr = CPLGetLastErrorType();
        int nLastErrNo = CPLGetLastErrorNo();
        CPLString osLastErrorMsg = CPLGetLastErrorMsg();
        CPLXMLNode *psXMLNode =
            CPLParseXMLString(oJP2Geo.pszGDALMultiDomainMetadata);
        if (CPLGetLastErrorType() == CE_None && eLastErr != CE_None)
            CPLErrorSetState(eLastErr, nLastErrNo, osLastErrorMsg.c_str());

        if (psXMLNode)
        {
            GDALMultiDomainMetadata oLocalMDMD;
            oLocalMDMD.XMLInit(psXMLNode, FALSE);
            GDALDataset::SetMetadata(oLocalMDMD.GetMetadata());
            for (const char *pszDomain : cpl::Iterate(
                     static_cast<CSLConstList>(oLocalMDMD.GetDomainList())))
            {
                if (!EQUAL(pszDomain, "") &&
                    !EQUAL(pszDomain, "IMAGE_STRUCTURE"))
                {
                    if (GDALDataset::GetMetadata(pszDomain) != nullptr)
                    {
                        CPLDebug(
                            "GDALJP2",
                            "GDAL metadata overrides metadata in %s domain "
                            "over metadata read from other boxes",
                            pszDomain);
                    }
                    GDALDataset::SetMetadata(oLocalMDMD.GetMetadata(pszDomain),
                                             pszDomain);
                }
            }
            CPLDestroyXMLNode(psXMLNode);
        }
        else
        {
            CPLErrorReset();
        }
    }

    /* -------------------------------------------------------------------- */
    /*      Do we have other misc metadata (from resd box for now) ?        */
    /* -------------------------------------------------------------------- */
    if (oJP2Geo.papszMetadata != nullptr)
    {
        char **papszMD = CSLDuplicate(GDALDataset::GetMetadata());

        papszMD = CSLMerge(papszMD, oJP2Geo.papszMetadata);
        GDALDataset::SetMetadata(papszMD);

        CSLDestroy(papszMD);
    }

    /* -------------------------------------------------------------------- */
    /*      Do we have XML IPR ?                                            */
    /* -------------------------------------------------------------------- */
    if (oJP2Geo.pszXMLIPR != nullptr)
    {
        char *apszMD[2] = {oJP2Geo.pszXMLIPR, nullptr};
        GDALDataset::SetMetadata(apszMD, "xml:IPR");
    }

    /* -------------------------------------------------------------------- */
    /*      Check for world file.                                           */
    /* -------------------------------------------------------------------- */
    if (m_nWORLDFILEIndex >= 0 &&
        ((bGeoTransformValid &&
          m_nWORLDFILEIndex < m_nGeoTransformGeorefSrcIndex) ||
         !bGeoTransformValid))
    {
        bGeoTransformValid |=
            GDALReadWorldFile2(pszOverrideFilename, nullptr, m_gt,
                               poOpenInfo->GetSiblingFiles(),
                               &pszWldFilename) ||
            GDALReadWorldFile2(pszOverrideFilename, ".wld", m_gt,
                               poOpenInfo->GetSiblingFiles(), &pszWldFilename);
        if (bGeoTransformValid)
        {
            m_nGeoTransformGeorefSrcIndex = m_nWORLDFILEIndex;
            m_bPixelIsPoint = false;
            m_nPixelIsPointGeorefSrcIndex = -1;
        }
    }

    GDALMDReaderManager mdreadermanager;
    GDALMDReaderBase *mdreader = mdreadermanager.GetReader(
        poOpenInfo->pszFilename, poOpenInfo->GetSiblingFiles(), MDR_ANY);
    if (nullptr != mdreader)
    {
        mdreader->FillMetadata(&(oMDMD));
        papszMetadataFiles = mdreader->GetMetadataFiles();
    }
}

/************************************************************************/
/*                            GetFileList()                             */
/************************************************************************/

char **GDALJP2AbstractDataset::GetFileList()

{
    char **papszFileList = GDALGeorefPamDataset::GetFileList();

    if (pszWldFilename != nullptr &&
        m_nGeoTransformGeorefSrcIndex == m_nWORLDFILEIndex &&
        GDALCanReliablyUseSiblingFileList(pszWldFilename) &&
        CSLFindString(papszFileList, pszWldFilename) == -1)
    {
        GDALGeoTransform l_gt;
        GetGeoTransform(l_gt);
        // GetGeoTransform() can modify m_nGeoTransformGeorefSrcIndex
        // cppcheck-suppress knownConditionTrueFalse
        if (m_nGeoTransformGeorefSrcIndex == m_nWORLDFILEIndex)
        {
            papszFileList = CSLAddString(papszFileList, pszWldFilename);
        }
    }
    if (papszMetadataFiles != nullptr)
    {
        for (int i = 0; papszMetadataFiles[i] != nullptr; ++i)
        {
            papszFileList = CSLAddString(papszFileList, papszMetadataFiles[i]);
        }
    }
    return papszFileList;
}

/************************************************************************/
/*                        LoadVectorLayers()                            */
/************************************************************************/

void GDALJP2AbstractDataset::LoadVectorLayers(int bOpenRemoteResources)
{
    char **papszGMLJP2 = GetMetadata("xml:gml.root-instance");
    if (papszGMLJP2 == nullptr)
        return;

    CPLErr eLastErr = CPLGetLastErrorType();
    int nLastErrNo = CPLGetLastErrorNo();
    CPLString osLastErrorMsg = CPLGetLastErrorMsg();
    CPLXMLNode *const psRoot = CPLParseXMLString(papszGMLJP2[0]);
    if (CPLGetLastErrorType() == CE_None && eLastErr != CE_None)
        CPLErrorSetState(eLastErr, nLastErrNo, osLastErrorMsg.c_str());

    if (psRoot == nullptr)
        return;
    CPLXMLNode *const psCC =
        CPLGetXMLNode(psRoot, "=gmljp2:GMLJP2CoverageCollection");
    if (psCC == nullptr)
    {
        CPLDestroyXMLNode(psRoot);
        return;
    }

    const std::string osTmpDir = VSIMemGenerateHiddenFilename("gmljp2");

    // Find feature collections.
    int nLayersAtCC = 0;
    int nLayersAtGC = 0;
    // CPLXMLNode* psCCChildIter = psCC->psChild;
    for (CPLXMLNode *psCCChildIter = psCC->psChild; psCCChildIter != nullptr;
         psCCChildIter = psCCChildIter->psNext)
    {
        if (psCCChildIter->eType != CXT_Element ||
            strcmp(psCCChildIter->pszValue, "gmljp2:featureMember") != 0 ||
            psCCChildIter->psChild == nullptr ||
            psCCChildIter->psChild->eType != CXT_Element)
            continue;

        CPLXMLNode *const psGCorGMLJP2Features = psCCChildIter->psChild;
        bool bIsGC =
            strstr(psGCorGMLJP2Features->pszValue, "GridCoverage") != nullptr;

        for (CPLXMLNode *psGCorGMLJP2FeaturesChildIter =
                 psGCorGMLJP2Features->psChild;
             psGCorGMLJP2FeaturesChildIter != nullptr;
             psGCorGMLJP2FeaturesChildIter =
                 psGCorGMLJP2FeaturesChildIter->psNext)
        {
            if (psGCorGMLJP2FeaturesChildIter->eType != CXT_Element ||
                strcmp(psGCorGMLJP2FeaturesChildIter->pszValue,
                       "gmljp2:feature") != 0 ||
                psGCorGMLJP2FeaturesChildIter->psChild == nullptr)
                continue;

            CPLXMLNode *psFC = nullptr;
            bool bFreeFC = false;

            CPLXMLNode *const psChild = psGCorGMLJP2FeaturesChildIter->psChild;
            if (psChild->eType == CXT_Attribute &&
                strcmp(psChild->pszValue, "xlink:href") == 0 &&
                STARTS_WITH(psChild->psChild->pszValue, "gmljp2://xml/"))
            {
                const char *const pszBoxName =
                    psChild->psChild->pszValue + strlen("gmljp2://xml/");
                char **papszBoxData =
                    GetMetadata(CPLSPrintf("xml:%s", pszBoxName));
                if (papszBoxData != nullptr)
                {
                    psFC = CPLParseXMLString(papszBoxData[0]);
                    bFreeFC = true;
                }
                else
                {
                    CPLDebug("GMLJP2",
                             "gmljp2:feature references %s, "
                             "but no corresponding box found",
                             psChild->psChild->pszValue);
                }
            }

            CPLString osGMLTmpFile;
            if (psChild->eType == CXT_Attribute &&
                strcmp(psChild->pszValue, "xlink:href") == 0 &&
                (STARTS_WITH(psChild->psChild->pszValue, "http://") ||
                 STARTS_WITH(psChild->psChild->pszValue, "https://")))
            {
                if (!bOpenRemoteResources)
                    CPLDebug(
                        "GMLJP2",
                        "Remote feature collection %s mentioned in GMLJP2 box",
                        psChild->psChild->pszValue);
                else
                    osGMLTmpFile =
                        "/vsicurl/" + CPLString(psChild->psChild->pszValue);
            }
            else if (psChild->eType == CXT_Element &&
                     strstr(psChild->pszValue, "FeatureCollection") != nullptr)
            {
                psFC = psChild;
            }

            if (psFC == nullptr && osGMLTmpFile.empty())
            {
                continue;
            }

            if (psFC != nullptr)
            {
                osGMLTmpFile =
                    CPLFormFilenameSafe(osTmpDir.c_str(), "my.gml", nullptr);
                // Create temporary .gml file.
                CPLSerializeXMLTreeToFile(psFC, osGMLTmpFile);
            }

            CPLDebug("GMLJP2", "Found a FeatureCollection at %s level",
                     bIsGC ? "GridCoverage" : "CoverageCollection");

            CPLString osXSDTmpFile;

            if (psFC)
            {
                // Try to localize its .xsd schema in a GMLJP2 auxiliary box
                const char *const pszSchemaLocation =
                    CPLGetXMLValue(psFC, "xsi:schemaLocation", nullptr);
                if (pszSchemaLocation)
                {
                    char **papszTokens = CSLTokenizeString2(
                        pszSchemaLocation, " \t\n",
                        CSLT_HONOURSTRINGS | CSLT_STRIPLEADSPACES |
                            CSLT_STRIPENDSPACES);

                    if ((CSLCount(papszTokens) % 2) == 0)
                    {
                        for (char **papszIter = papszTokens;
                             *papszIter != nullptr; papszIter += 2)
                        {
                            if (STARTS_WITH(papszIter[1], "gmljp2://xml/"))
                            {
                                const char *pszBoxName =
                                    papszIter[1] + strlen("gmljp2://xml/");
                                char **papszBoxData = GetMetadata(
                                    CPLSPrintf("xml:%s", pszBoxName));
                                if (papszBoxData != nullptr)
                                {
                                    osXSDTmpFile = CPLFormFilenameSafe(
                                        osTmpDir.c_str(), "my.xsd", nullptr);
                                    CPL_IGNORE_RET_VAL(
                                        VSIFCloseL(VSIFileFromMemBuffer(
                                            osXSDTmpFile,
                                            reinterpret_cast<GByte *>(
                                                papszBoxData[0]),
                                            strlen(papszBoxData[0]), FALSE)));
                                }
                                else
                                {
                                    CPLDebug(
                                        "GMLJP2",
                                        "Feature collection references %s, "
                                        "but no corresponding box found",
                                        papszIter[1]);
                                }
                                break;
                            }
                        }
                    }
                    CSLDestroy(papszTokens);
                }
                if (bFreeFC)
                {
                    CPLDestroyXMLNode(psFC);
                    psFC = nullptr;
                }
            }

            GDALDriverH hDrv = GDALIdentifyDriver(osGMLTmpFile, nullptr);
            GDALDriverH hGMLDrv = GDALGetDriverByName("GML");
            if (hDrv != nullptr && hDrv == hGMLDrv)
            {
                char *apszOpenOptions[2] = {
                    const_cast<char *>("FORCE_SRS_DETECTION=YES"), nullptr};
                GDALDatasetUniquePtr poTmpDS(
                    GDALDataset::Open(osGMLTmpFile, GDAL_OF_VECTOR, nullptr,
                                      apszOpenOptions, nullptr));
                if (poTmpDS)
                {
                    int nLayers = poTmpDS->GetLayerCount();
                    for (int i = 0; i < nLayers; ++i)
                    {
                        if (poMemDS == nullptr)
                            poMemDS = MEMDataset::Create("", 0, 0, 0,
                                                         GDT_Unknown, nullptr);
                        OGRLayer *poSrcLyr = poTmpDS->GetLayer(i);
                        const char *const pszLayerName =
                            bIsGC
                                ? CPLSPrintf("FC_GridCoverage_%d_%s",
                                             ++nLayersAtGC, poSrcLyr->GetName())
                                : CPLSPrintf("FC_CoverageCollection_%d_%s",
                                             ++nLayersAtCC,
                                             poSrcLyr->GetName());
                        poMemDS->CopyLayer(poSrcLyr, pszLayerName, nullptr);
                    }
                }
            }
            else
            {
                CPLDebug("GMLJP2",
                         "No GML driver found to read feature collection");
            }

            VSIRmdirRecursive(osTmpDir.c_str());
        }
    }

    // Find annotations
    int nAnnotations = 0;
    for (CPLXMLNode *psCCChildIter = psCC->psChild; psCCChildIter != nullptr;
         psCCChildIter = psCCChildIter->psNext)
    {
        if (psCCChildIter->eType != CXT_Element ||
            strcmp(psCCChildIter->pszValue, "gmljp2:featureMember") != 0 ||
            psCCChildIter->psChild == nullptr ||
            psCCChildIter->psChild->eType != CXT_Element)
            continue;
        CPLXMLNode *const psGCorGMLJP2Features = psCCChildIter->psChild;
        bool bIsGC =
            strstr(psGCorGMLJP2Features->pszValue, "GridCoverage") != nullptr;
        if (!bIsGC)
            continue;
        for (CPLXMLNode *psGCorGMLJP2FeaturesChildIter =
                 psGCorGMLJP2Features->psChild;
             psGCorGMLJP2FeaturesChildIter != nullptr;
             psGCorGMLJP2FeaturesChildIter =
                 psGCorGMLJP2FeaturesChildIter->psNext)
        {
            if (psGCorGMLJP2FeaturesChildIter->eType != CXT_Element ||
                strcmp(psGCorGMLJP2FeaturesChildIter->pszValue,
                       "gmljp2:annotation") != 0 ||
                psGCorGMLJP2FeaturesChildIter->psChild == nullptr ||
                psGCorGMLJP2FeaturesChildIter->psChild->eType != CXT_Element ||
                strstr(psGCorGMLJP2FeaturesChildIter->psChild->pszValue,
                       "kml") == nullptr)
                continue;

            CPLDebug("GMLJP2", "Found a KML annotation");

            // Create temporary .kml file.
            CPLXMLNode *const psKML = psGCorGMLJP2FeaturesChildIter->psChild;
            const CPLString osKMLTmpFile(
                VSIMemGenerateHiddenFilename("my.kml"));
            CPLSerializeXMLTreeToFile(psKML, osKMLTmpFile);

            GDALDatasetUniquePtr poTmpDS(GDALDataset::Open(
                osKMLTmpFile, GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
            if (poTmpDS)
            {
                int nLayers = poTmpDS->GetLayerCount();
                for (int i = 0; i < nLayers; ++i)
                {
                    if (poMemDS == nullptr)
                        poMemDS = MEMDataset::Create("", 0, 0, 0, GDT_Unknown,
                                                     nullptr);
                    OGRLayer *const poSrcLyr = poTmpDS->GetLayer(i);
                    const char *pszLayerName =
                        CPLSPrintf("Annotation_%d_%s", ++nAnnotations,
                                   poSrcLyr->GetName());
                    poMemDS->CopyLayer(poSrcLyr, pszLayerName, nullptr);
                }
            }
            else
            {
                CPLDebug("GMLJP2",
                         "No KML/LIBKML driver found to read annotation");
            }

            VSIUnlink(osKMLTmpFile);
        }
    }

    CPLDestroyXMLNode(psRoot);
}

/************************************************************************/
/*                           GetLayerCount()                            */
/************************************************************************/

int GDALJP2AbstractDataset::GetLayerCount()
{
    return poMemDS != nullptr ? poMemDS->GetLayerCount() : 0;
}

/************************************************************************/
/*                             GetLayer()                               */
/************************************************************************/

OGRLayer *GDALJP2AbstractDataset::GetLayer(int i)
{
    return poMemDS != nullptr ? poMemDS->GetLayer(i) : nullptr;
}

/************************************************************************/
/*                            GetMetadata()                             */
/************************************************************************/

char **GDALJP2AbstractDataset::GetMetadata(const char *pszDomain)
{
    if (pszDomain && EQUAL(pszDomain, "IMAGE_STRUCTURE"))
    {
        if (m_aosImageStructureMetadata.empty())
        {
            VSILFILE *fp = GetFileHandle();
            m_aosImageStructureMetadata.Assign(
                CSLDuplicate(GDALGeorefPamDataset::GetMetadata(pszDomain)),
                true);
            CPLErrorStateBackuper oErrorStateBackuper(CPLQuietErrorHandler);
            const char *pszReversibility =
                GDALGetJPEG2000Reversibility(GetDescription(), fp);
            if (pszReversibility)
                m_aosImageStructureMetadata.SetNameValue(
                    "COMPRESSION_REVERSIBILITY", pszReversibility);
        }
        return m_aosImageStructureMetadata.List();
    }
    return GDALGeorefPamDataset::GetMetadata(pszDomain);
}

/************************************************************************/
/*                        GetMetadataItem()                             */
/************************************************************************/

const char *GDALJP2AbstractDataset::GetMetadataItem(const char *pszName,
                                                    const char *pszDomain)
{
    if (pszDomain && EQUAL(pszDomain, "IMAGE_STRUCTURE") &&
        EQUAL(pszName, "COMPRESSION_REVERSIBILITY"))
    {
        char **papszMD = GetMetadata(pszDomain);
        return CSLFetchNameValue(papszMD, pszName);
    }
    return GDALGeorefPamDataset::GetMetadataItem(pszName, pszDomain);
}

/*! @endcond */
