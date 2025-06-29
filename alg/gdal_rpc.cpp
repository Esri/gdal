/******************************************************************************
 *
 * Project:  Image Warper
 * Purpose:  Implements a rational polynomial (RPC) based transformer.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2003, Frank Warmerdam <warmerdam@pobox.com>
 * Copyright (c) 2009-2013, Even Rouault <even dot rouault at spatialys.com>
 *
 * SPDX-License-Identifier: MIT
 ****************************************************************************/

#include "cpl_port.h"
#include "gdal_alg.h"

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <limits>
#include <string>

#include "cpl_conv.h"
#include "cpl_error.h"
#include "cpl_mem_cache.h"
#include "cpl_minixml.h"
#include "cpl_string.h"
#include "cpl_vsi.h"
#include "gdal.h"
#include "gdal_interpolateatpoint.h"
#include "gdal_mdreader.h"
#include "gdal_alg_priv.h"
#include "gdal_priv.h"

#ifdef USE_NEON_OPTIMIZATIONS
#define USE_SSE2
#elif defined(__x86_64) || defined(_M_X64)
#define USE_SSE2
#endif

#ifdef USE_SSE2
#include "gdalsse_priv.h"
#define USE_SSE2_OPTIM
#endif

#include "ogr_api.h"
#include "ogr_geometry.h"
#include "ogr_spatialref.h"
#include "ogr_srs_api.h"
#include "gdalresamplingkernels.h"

// #define DEBUG_VERBOSE_EXTRACT_DEM

CPL_C_START
CPLXMLNode *GDALSerializeRPCTransformer(void *pTransformArg);
void *GDALDeserializeRPCTransformer(CPLXMLNode *psTree);
CPL_C_END

constexpr int MAX_ABS_VALUE_WARNINGS = 20;
constexpr double DEFAULT_PIX_ERR_THRESHOLD = 0.1;

/************************************************************************/
/*                            RPCInfoToMD()                             */
/*                                                                      */
/*      Turn an RPCInfo structure back into its metadata format.        */
/************************************************************************/

char **RPCInfoV1ToMD(GDALRPCInfoV1 *psRPCInfo)

{
    GDALRPCInfoV2 sRPCInfo;
    memcpy(&sRPCInfo, psRPCInfo, sizeof(GDALRPCInfoV1));
    sRPCInfo.dfERR_BIAS = std::numeric_limits<double>::quiet_NaN();
    sRPCInfo.dfERR_RAND = std::numeric_limits<double>::quiet_NaN();
    return RPCInfoV2ToMD(&sRPCInfo);
}

char **RPCInfoV2ToMD(GDALRPCInfoV2 *psRPCInfo)

{
    char **papszMD = nullptr;
    CPLString osField, osMultiField;

    if (!std::isnan(psRPCInfo->dfERR_BIAS))
    {
        osField.Printf("%.15g", psRPCInfo->dfERR_BIAS);
        papszMD = CSLSetNameValue(papszMD, RPC_ERR_BIAS, osField);
    }

    if (!std::isnan(psRPCInfo->dfERR_RAND))
    {
        osField.Printf("%.15g", psRPCInfo->dfERR_RAND);
        papszMD = CSLSetNameValue(papszMD, RPC_ERR_RAND, osField);
    }

    osField.Printf("%.15g", psRPCInfo->dfLINE_OFF);
    papszMD = CSLSetNameValue(papszMD, RPC_LINE_OFF, osField);

    osField.Printf("%.15g", psRPCInfo->dfSAMP_OFF);
    papszMD = CSLSetNameValue(papszMD, RPC_SAMP_OFF, osField);

    osField.Printf("%.15g", psRPCInfo->dfLAT_OFF);
    papszMD = CSLSetNameValue(papszMD, RPC_LAT_OFF, osField);

    osField.Printf("%.15g", psRPCInfo->dfLONG_OFF);
    papszMD = CSLSetNameValue(papszMD, RPC_LONG_OFF, osField);

    osField.Printf("%.15g", psRPCInfo->dfHEIGHT_OFF);
    papszMD = CSLSetNameValue(papszMD, RPC_HEIGHT_OFF, osField);

    osField.Printf("%.15g", psRPCInfo->dfLINE_SCALE);
    papszMD = CSLSetNameValue(papszMD, RPC_LINE_SCALE, osField);

    osField.Printf("%.15g", psRPCInfo->dfSAMP_SCALE);
    papszMD = CSLSetNameValue(papszMD, RPC_SAMP_SCALE, osField);

    osField.Printf("%.15g", psRPCInfo->dfLAT_SCALE);
    papszMD = CSLSetNameValue(papszMD, RPC_LAT_SCALE, osField);

    osField.Printf("%.15g", psRPCInfo->dfLONG_SCALE);
    papszMD = CSLSetNameValue(papszMD, RPC_LONG_SCALE, osField);

    osField.Printf("%.15g", psRPCInfo->dfHEIGHT_SCALE);
    papszMD = CSLSetNameValue(papszMD, RPC_HEIGHT_SCALE, osField);

    osField.Printf("%.15g", psRPCInfo->dfMIN_LONG);
    papszMD = CSLSetNameValue(papszMD, RPC_MIN_LONG, osField);

    osField.Printf("%.15g", psRPCInfo->dfMIN_LAT);
    papszMD = CSLSetNameValue(papszMD, RPC_MIN_LAT, osField);

    osField.Printf("%.15g", psRPCInfo->dfMAX_LONG);
    papszMD = CSLSetNameValue(papszMD, RPC_MAX_LONG, osField);

    osField.Printf("%.15g", psRPCInfo->dfMAX_LAT);
    papszMD = CSLSetNameValue(papszMD, RPC_MAX_LAT, osField);

    for (int i = 0; i < 20; i++)
    {
        osField.Printf("%.15g", psRPCInfo->adfLINE_NUM_COEFF[i]);
        if (i > 0)
            osMultiField += " ";
        else
            osMultiField = "";
        osMultiField += osField;
    }
    papszMD = CSLSetNameValue(papszMD, "LINE_NUM_COEFF", osMultiField);

    for (int i = 0; i < 20; i++)
    {
        osField.Printf("%.15g", psRPCInfo->adfLINE_DEN_COEFF[i]);
        if (i > 0)
            osMultiField += " ";
        else
            osMultiField = "";
        osMultiField += osField;
    }
    papszMD = CSLSetNameValue(papszMD, "LINE_DEN_COEFF", osMultiField);

    for (int i = 0; i < 20; i++)
    {
        osField.Printf("%.15g", psRPCInfo->adfSAMP_NUM_COEFF[i]);
        if (i > 0)
            osMultiField += " ";
        else
            osMultiField = "";
        osMultiField += osField;
    }
    papszMD = CSLSetNameValue(papszMD, "SAMP_NUM_COEFF", osMultiField);

    for (int i = 0; i < 20; i++)
    {
        osField.Printf("%.15g", psRPCInfo->adfSAMP_DEN_COEFF[i]);
        if (i > 0)
            osMultiField += " ";
        else
            osMultiField = "";
        osMultiField += osField;
    }
    papszMD = CSLSetNameValue(papszMD, "SAMP_DEN_COEFF", osMultiField);

    return papszMD;
}

/************************************************************************/
/*                          RPCComputeTerms()                           */
/************************************************************************/

static void RPCComputeTerms(double dfLong, double dfLat, double dfHeight,
                            double *padfTerms)

{
    padfTerms[0] = 1.0;
    padfTerms[1] = dfLong;
    padfTerms[2] = dfLat;
    padfTerms[3] = dfHeight;
    padfTerms[4] = dfLong * dfLat;
    padfTerms[5] = dfLong * dfHeight;
    padfTerms[6] = dfLat * dfHeight;
    padfTerms[7] = dfLong * dfLong;
    padfTerms[8] = dfLat * dfLat;
    padfTerms[9] = dfHeight * dfHeight;

    padfTerms[10] = dfLong * dfLat * dfHeight;
    padfTerms[11] = dfLong * dfLong * dfLong;
    padfTerms[12] = dfLong * dfLat * dfLat;
    padfTerms[13] = dfLong * dfHeight * dfHeight;
    padfTerms[14] = dfLong * dfLong * dfLat;
    padfTerms[15] = dfLat * dfLat * dfLat;
    padfTerms[16] = dfLat * dfHeight * dfHeight;
    padfTerms[17] = dfLong * dfLong * dfHeight;
    padfTerms[18] = dfLat * dfLat * dfHeight;
    padfTerms[19] = dfHeight * dfHeight * dfHeight;
}

/************************************************************************/
/* ==================================================================== */
/*                           GDALRPCTransformer                         */
/* ==================================================================== */
/************************************************************************/

/*! DEM Resampling Algorithm */
typedef enum
{
    /*! Nearest neighbour (select on one input pixel) */ DRA_NearestNeighbour =
        0,
    /*! Bilinear (2x2 kernel) */ DRA_Bilinear = 1,
    /*! Cubic Convolution Approximation (4x4 kernel) */ DRA_CubicSpline = 2
} DEMResampleAlg;

typedef struct
{

    GDALTransformerInfo sTI;

    GDALRPCInfoV2 sRPC;

    double adfPLToLatLongGeoTransform[6];
    double dfRefZ;

    int bReversed;

    double dfPixErrThreshold;

    double dfHeightOffset;

    double dfHeightScale;

    char *pszDEMPath;

    DEMResampleAlg eResampleAlg;

    int bHasDEMMissingValue;
    double dfDEMMissingValue;
    char *pszDEMSRS;
    int bApplyDEMVDatumShift;

    GDALDataset *poDS;
    // the key is (nYBlock << 32) | nXBlock)
    lru11::Cache<uint64_t, std::shared_ptr<std::vector<double>>> *poCacheDEM;

    OGRCoordinateTransformation *poCT;

    int nMaxIterations;

    double adfDEMGeoTransform[6];
    double adfDEMReverseGeoTransform[6];

#ifdef USE_SSE2_OPTIM
    double adfDoubles[20 * 4 + 1];
    // LINE_NUM_COEFF, LINE_DEN_COEFF, SAMP_NUM_COEFF and then SAMP_DEN_COEFF.
    double *padfCoeffs;
#endif

    bool bRPCInverseVerbose;
    char *pszRPCInverseLog;

    char *pszRPCFootprint;
    OGRGeometry *poRPCFootprintGeom;
    OGRPreparedGeometry *poRPCFootprintPreparedGeom;

} GDALRPCTransformInfo;

static bool GDALRPCOpenDEM(GDALRPCTransformInfo *psTransform);

/************************************************************************/
/*                            RPCEvaluate()                             */
/************************************************************************/
#ifdef USE_SSE2_OPTIM

static void RPCEvaluate4(const double *padfTerms, const double *padfCoefs,
                         double &dfSum1, double &dfSum2, double &dfSum3,
                         double &dfSum4)

{
    XMMReg2Double sum1 = XMMReg2Double::Zero();
    XMMReg2Double sum2 = XMMReg2Double::Zero();
    XMMReg2Double sum3 = XMMReg2Double::Zero();
    XMMReg2Double sum4 = XMMReg2Double::Zero();
    for (int i = 0; i < 20; i += 2)
    {
        const XMMReg2Double terms =
            XMMReg2Double::Load2ValAligned(padfTerms + i);

        // LINE_NUM_COEFF.
        const XMMReg2Double coefs1 =
            XMMReg2Double::Load2ValAligned(padfCoefs + i);

        // LINE_DEN_COEFF.
        const XMMReg2Double coefs2 =
            XMMReg2Double::Load2ValAligned(padfCoefs + i + 20);

        // SAMP_NUM_COEFF.
        const XMMReg2Double coefs3 =
            XMMReg2Double::Load2ValAligned(padfCoefs + i + 40);

        // SAMP_DEN_COEFF.
        const XMMReg2Double coefs4 =
            XMMReg2Double::Load2ValAligned(padfCoefs + i + 60);

        sum1 += terms * coefs1;
        sum2 += terms * coefs2;
        sum3 += terms * coefs3;
        sum4 += terms * coefs4;
    }
    dfSum1 = sum1.GetHorizSum();
    dfSum2 = sum2.GetHorizSum();
    dfSum3 = sum3.GetHorizSum();
    dfSum4 = sum4.GetHorizSum();
}

#else

static double RPCEvaluate(const double *padfTerms, const double *padfCoefs)

{
    double dfSum1 = 0.0;
    double dfSum2 = 0.0;

    for (int i = 0; i < 20; i += 2)
    {
        dfSum1 += padfTerms[i] * padfCoefs[i];
        dfSum2 += padfTerms[i + 1] * padfCoefs[i + 1];
    }

    return dfSum1 + dfSum2;
}

#endif

/************************************************************************/
/*                         RPCTransformPoint()                          */
/************************************************************************/

static void RPCTransformPoint(const GDALRPCTransformInfo *psRPCTransformInfo,
                              double dfLong, double dfLat, double dfHeight,
                              double *pdfPixel, double *pdfLine)

{
    double adfTermsWithMargin[20 + 1] = {};
    // Make padfTerms aligned on 16-byte boundary for SSE2 aligned loads.
    double *padfTerms =
        adfTermsWithMargin +
        (reinterpret_cast<GUIntptr_t>(adfTermsWithMargin) % 16) / 8;

    // Avoid dateline issues.
    double diffLong = dfLong - psRPCTransformInfo->sRPC.dfLONG_OFF;
    if (diffLong < -270)
    {
        diffLong += 360;
    }
    else if (diffLong > 270)
    {
        diffLong -= 360;
    }

    const double dfNormalizedLong =
        diffLong / psRPCTransformInfo->sRPC.dfLONG_SCALE;
    const double dfNormalizedLat =
        (dfLat - psRPCTransformInfo->sRPC.dfLAT_OFF) /
        psRPCTransformInfo->sRPC.dfLAT_SCALE;
    const double dfNormalizedHeight =
        (dfHeight - psRPCTransformInfo->sRPC.dfHEIGHT_OFF) /
        psRPCTransformInfo->sRPC.dfHEIGHT_SCALE;

    // The absolute values of the 3 above normalized values are supposed to be
    // below 1. Warn (as debug message) if it is not the case. We allow for some
    // margin above 1 (1.5, somewhat arbitrary chosen) before warning.
    static int nCountWarningsAboutAboveOneNormalizedValues = 0;
    if (nCountWarningsAboutAboveOneNormalizedValues < MAX_ABS_VALUE_WARNINGS)
    {
        bool bWarned = false;
        if (fabs(dfNormalizedLong) > 1.5)
        {
            bWarned = true;
            CPLDebug(
                "RPC",
                "Normalized %s for (lon,lat,height)=(%f,%f,%f) is %f, "
                "i.e. with an absolute value of > 1, which may cause numeric "
                "stability problems",
                "longitude", dfLong, dfLat, dfHeight, dfNormalizedLong);
        }
        if (fabs(dfNormalizedLat) > 1.5)
        {
            bWarned = true;
            CPLDebug(
                "RPC",
                "Normalized %s for (lon,lat,height)=(%f,%f,%f) is %f, "
                "ie with an absolute value of > 1, which may cause numeric "
                "stability problems",
                "latitude", dfLong, dfLat, dfHeight, dfNormalizedLat);
        }
        if (fabs(dfNormalizedHeight) > 1.5)
        {
            bWarned = true;
            CPLDebug(
                "RPC",
                "Normalized %s for (lon,lat,height)=(%f,%f,%f) is %f, "
                "i.e. with an absolute value of > 1, which may cause numeric "
                "stability problems",
                "height", dfLong, dfLat, dfHeight, dfNormalizedHeight);
        }
        if (bWarned)
        {
            // Limit the number of warnings.
            nCountWarningsAboutAboveOneNormalizedValues++;
            if (nCountWarningsAboutAboveOneNormalizedValues ==
                MAX_ABS_VALUE_WARNINGS)
            {
                CPLDebug("RPC", "No more such debug warnings will be emitted");
            }
        }
    }

    RPCComputeTerms(dfNormalizedLong, dfNormalizedLat, dfNormalizedHeight,
                    padfTerms);

#ifdef USE_SSE2_OPTIM
    double dfSampNum = 0.0;
    double dfSampDen = 0.0;
    double dfLineNum = 0.0;
    double dfLineDen = 0.0;
    RPCEvaluate4(padfTerms, psRPCTransformInfo->padfCoeffs, dfLineNum,
                 dfLineDen, dfSampNum, dfSampDen);
    const double dfResultX = dfSampNum / dfSampDen;
    const double dfResultY = dfLineNum / dfLineDen;
#else
    const double dfResultX =
        RPCEvaluate(padfTerms, psRPCTransformInfo->sRPC.adfSAMP_NUM_COEFF) /
        RPCEvaluate(padfTerms, psRPCTransformInfo->sRPC.adfSAMP_DEN_COEFF);

    const double dfResultY =
        RPCEvaluate(padfTerms, psRPCTransformInfo->sRPC.adfLINE_NUM_COEFF) /
        RPCEvaluate(padfTerms, psRPCTransformInfo->sRPC.adfLINE_DEN_COEFF);
#endif

    // RPCs are using the center of upper left pixel = 0,0 convention
    // convert to top left corner = 0,0 convention used in GDAL.
    *pdfPixel = dfResultX * psRPCTransformInfo->sRPC.dfSAMP_SCALE +
                psRPCTransformInfo->sRPC.dfSAMP_OFF + 0.5;
    *pdfLine = dfResultY * psRPCTransformInfo->sRPC.dfLINE_SCALE +
               psRPCTransformInfo->sRPC.dfLINE_OFF + 0.5;
}

/************************************************************************/
/*                     GDALSerializeRPCDEMResample()                    */
/************************************************************************/

static const char *GDALSerializeRPCDEMResample(DEMResampleAlg eResampleAlg)
{
    switch (eResampleAlg)
    {
        case DRA_NearestNeighbour:
            return "near";
        case DRA_CubicSpline:
            return "cubic";
        default:
        case DRA_Bilinear:
            return "bilinear";
    }
}

/************************************************************************/
/*                   GDALCreateSimilarRPCTransformer()                  */
/************************************************************************/

static void *GDALCreateSimilarRPCTransformer(void *hTransformArg,
                                             double dfRatioX, double dfRatioY)
{
    VALIDATE_POINTER1(hTransformArg, "GDALCreateSimilarRPCTransformer",
                      nullptr);

    GDALRPCTransformInfo *psInfo =
        static_cast<GDALRPCTransformInfo *>(hTransformArg);

    GDALRPCInfoV2 sRPC;
    memcpy(&sRPC, &(psInfo->sRPC), sizeof(GDALRPCInfoV2));

    if (dfRatioX != 1.0 || dfRatioY != 1.0)
    {
        sRPC.dfLINE_OFF /= dfRatioY;
        sRPC.dfLINE_SCALE /= dfRatioY;
        sRPC.dfSAMP_OFF /= dfRatioX;
        sRPC.dfSAMP_SCALE /= dfRatioX;
    }

    char **papszOptions = nullptr;
    papszOptions = CSLSetNameValue(papszOptions, "RPC_HEIGHT",
                                   CPLSPrintf("%.17g", psInfo->dfHeightOffset));
    papszOptions = CSLSetNameValue(papszOptions, "RPC_HEIGHT_SCALE",
                                   CPLSPrintf("%.17g", psInfo->dfHeightScale));
    if (psInfo->pszDEMPath != nullptr)
    {
        papszOptions =
            CSLSetNameValue(papszOptions, "RPC_DEM", psInfo->pszDEMPath);
        papszOptions =
            CSLSetNameValue(papszOptions, "RPC_DEMINTERPOLATION",
                            GDALSerializeRPCDEMResample(psInfo->eResampleAlg));
        if (psInfo->bHasDEMMissingValue)
            papszOptions =
                CSLSetNameValue(papszOptions, "RPC_DEM_MISSING_VALUE",
                                CPLSPrintf("%.17g", psInfo->dfDEMMissingValue));
        papszOptions =
            CSLSetNameValue(papszOptions, "RPC_DEM_APPLY_VDATUM_SHIFT",
                            (psInfo->bApplyDEMVDatumShift) ? "TRUE" : "FALSE");
    }
    papszOptions = CSLSetNameValue(papszOptions, "RPC_MAX_ITERATIONS",
                                   CPLSPrintf("%d", psInfo->nMaxIterations));

    GDALRPCTransformInfo *psNewInfo =
        static_cast<GDALRPCTransformInfo *>(GDALCreateRPCTransformerV2(
            &sRPC, psInfo->bReversed, psInfo->dfPixErrThreshold, papszOptions));
    CSLDestroy(papszOptions);

    return psNewInfo;
}

/************************************************************************/
/*                      GDALRPCGetHeightAtLongLat()                     */
/************************************************************************/

static int GDALRPCGetDEMHeight(GDALRPCTransformInfo *psTransform,
                               const double dfXIn, const double dfYIn,
                               double *pdfDEMH);

static bool GDALRPCGetHeightAtLongLat(GDALRPCTransformInfo *psTransform,
                                      const double dfXIn, const double dfYIn,
                                      double *pdfHeight,
                                      double *pdfDEMPixel = nullptr,
                                      double *pdfDEMLine = nullptr)
{
    double dfVDatumShift = 0.0;
    double dfDEMH = 0.0;
    if (psTransform->poDS)
    {
        double dfX = 0.0;
        double dfY = 0.0;
        double dfXTemp = dfXIn;
        double dfYTemp = dfYIn;
        // Check if dem is not in WGS84 and transform points padfX[i], padfY[i].
        if (psTransform->poCT)
        {
            double dfZ = 0.0;
            if (!psTransform->poCT->Transform(1, &dfXTemp, &dfYTemp, &dfZ))
            {
                return false;
            }

            // We must take the opposite since poCT transforms from
            // WGS84 to geoid. And we are going to do the reverse:
            // take an elevation over the geoid and transforms it to WGS84.
            if (psTransform->bApplyDEMVDatumShift)
                dfVDatumShift = -dfZ;
        }

        bool bRetried = false;
    retry:
        GDALApplyGeoTransform(psTransform->adfDEMReverseGeoTransform, dfXTemp,
                              dfYTemp, &dfX, &dfY);
        if (pdfDEMPixel)
            *pdfDEMPixel = dfX;
        if (pdfDEMLine)
            *pdfDEMLine = dfY;

        if (!GDALRPCGetDEMHeight(psTransform, dfX, dfY, &dfDEMH))
        {
            // Try to handle the case where the DEM is in LL WGS84 and spans
            // over [-180,180], (or very close to it ), presumably with much
            // hole in the middle if using VRT, and the longitude goes beyond
            // that interval.
            if (!bRetried && psTransform->poCT == nullptr &&
                (dfXIn >= 180.0 || dfXIn <= -180.0))
            {
                const int nRasterXSize = psTransform->poDS->GetRasterXSize();
                const double dfMinDEMLong = psTransform->adfDEMGeoTransform[0];
                const double dfMaxDEMLong =
                    psTransform->adfDEMGeoTransform[0] +
                    nRasterXSize * psTransform->adfDEMGeoTransform[1];
                if (fabs(dfMinDEMLong - -180) < 0.1 &&
                    fabs(dfMaxDEMLong - 180) < 0.1)
                {
                    if (dfXIn >= 180)
                    {
                        dfXTemp = dfXIn - 360;
                        dfYTemp = dfYIn;
                    }
                    else
                    {
                        dfXTemp = dfXIn + 360;
                        dfYTemp = dfYIn;
                    }
                    bRetried = true;
                    goto retry;
                }
            }

            if (psTransform->bHasDEMMissingValue)
                dfDEMH = psTransform->dfDEMMissingValue;
            else
            {
                return false;
            }
        }
#ifdef DEBUG_VERBOSE_EXTRACT_DEM
        CPLDebug("RPC_DEM", "X=%f, Y=%f -> Z=%f", dfX, dfY, dfDEMH);
#endif
    }

    *pdfHeight = dfVDatumShift + (psTransform->dfHeightOffset +
                                  dfDEMH * psTransform->dfHeightScale);
    return true;
}

/************************************************************************/
/*                      GDALCreateRPCTransformer()                      */
/************************************************************************/

void *GDALCreateRPCTransformerV1(GDALRPCInfoV1 *psRPCInfo, int bReversed,
                                 double dfPixErrThreshold, char **papszOptions)

{
    GDALRPCInfoV2 sRPCInfo;
    memcpy(&sRPCInfo, psRPCInfo, sizeof(GDALRPCInfoV1));
    sRPCInfo.dfERR_BIAS = std::numeric_limits<double>::quiet_NaN();
    sRPCInfo.dfERR_RAND = std::numeric_limits<double>::quiet_NaN();
    return GDALCreateRPCTransformerV2(&sRPCInfo, bReversed, dfPixErrThreshold,
                                      papszOptions);
}

/**
 * Create an RPC based transformer.
 *
 * The geometric sensor model describing the physical relationship between
 * image coordinates and ground coordinates is known as a Rigorous Projection
 * Model. A Rigorous Projection Model expresses the mapping of the image space
 * coordinates of rows and columns (r,c) onto the object space reference
 * surface geodetic coordinates (long, lat, height).
 *
 * A RPC supports a generic description of the Rigorous Projection Models. The
 * approximation used by GDAL (RPC00) is a set of rational polynomials
 * expressing the normalized row and column values, (rn , cn), as a function of
 * normalized geodetic latitude, longitude, and height, (P, L, H), given a
 * set of normalized polynomial coefficients (LINE_NUM_COEF_n, LINE_DEN_COEF_n,
 * SAMP_NUM_COEF_n, SAMP_DEN_COEF_n). Normalized values, rather than actual
 * values are used in order to minimize introduction of errors during the
 * calculations. The transformation between row and column values (r,c), and
 * normalized row and column values (rn, cn), and between the geodetic
 * latitude, longitude, and height and normalized geodetic latitude,
 * longitude, and height (P, L, H), is defined by a set of normalizing
 * translations (offsets) and scales that ensure all values are contained in
 * the range -1 to +1.
 *
 * This function creates a GDALTransformFunc compatible transformer
 * for going between image pixel/line and long/lat/height coordinates
 * using RPCs.  The RPCs are provided in a GDALRPCInfo structure which is
 * normally read from metadata using GDALExtractRPCInfo().
 *
 * GDAL RPC Metadata has the following entries (also described in GDAL RFC 22
 * and the GeoTIFF RPC document http://geotiff.maptools.org/rpc_prop.html .
 *
 * <ul>
 * <li>ERR_BIAS: Error - Bias. The RMS bias error in meters per horizontal axis
 * of all points in the image (-1.0 if unknown)
 * <li>ERR_RAND: Error - Random. RMS random error in meters per horizontal axis
 * of each point in the image (-1.0 if unknown)
 * <li>LINE_OFF: Line Offset
 * <li>SAMP_OFF: Sample Offset
 * <li>LAT_OFF: Geodetic Latitude Offset
 * <li>LONG_OFF: Geodetic Longitude Offset
 * <li>HEIGHT_OFF: Geodetic Height Offset
 * <li>LINE_SCALE: Line Scale
 * <li>SAMP_SCALE: Sample Scale
 * <li>LAT_SCALE: Geodetic Latitude Scale
 * <li>LONG_SCALE: Geodetic Longitude Scale
 * <li>HEIGHT_SCALE: Geodetic Height Scale

 * <li>LINE_NUM_COEFF (1-20): Line Numerator Coefficients. Twenty coefficients
 * for the polynomial in the Numerator of the rn equation. (space separated)
 * <li>LINE_DEN_COEFF (1-20): Line Denominator Coefficients. Twenty coefficients
 * for the polynomial in the Denominator of the rn equation. (space separated)
 * <li>SAMP_NUM_COEFF (1-20): Sample Numerator Coefficients. Twenty coefficients
 * for the polynomial in the Numerator of the cn equation. (space separated)
 * <li>SAMP_DEN_COEFF (1-20): Sample Denominator Coefficients. Twenty
 * coefficients for the polynomial in the Denominator of the cn equation. (space
 * separated)
 * </ul>
 *
 * Some drivers (such as DIMAP) may also fill a HEIGHT_DEFAULT item that can be
 * used by GDALCreateGenImgProjTransformer2() to initialize the below RPC_HEIGHT
 * transformer option if none of RPC_HEIGHT and RPC_DEM are specified.
 * Otherwise, if none of RPC_HEIGHT and RPC_DEM are specified as transformer
 * options and if HEIGHT_DEFAULT is no available, a height of 0 will be used.
 *
 * The transformer normally maps from pixel/line/height to long/lat/height space
 * as a forward transformation though in RPC terms that would be considered
 * an inverse transformation (and is solved by iterative approximation using
 * long/lat/height to pixel/line transformations).  The default direction can
 * be reversed by passing bReversed=TRUE.
 *
 * The iterative solution of pixel/line
 * to lat/long/height is currently run for up to 10 iterations or until
 * the apparent error is less than dfPixErrThreshold pixels.  Passing zero
 * will not avoid all error, but will cause the operation to run for the maximum
 * number of iterations.
 *
 * Starting with GDAL 2.1, debugging of the RPC inverse transformer can be done
 * by setting the RPC_INVERSE_VERBOSE configuration option to YES (in which case
 * extra debug information will be displayed in the "RPC" debug category, so
 * requiring CPL_DEBUG to be also set) and/or by setting RPC_INVERSE_LOG to a
 * filename that will contain the content of iterations (this last option only
 * makes sense when debugging point by point, since each time
 * RPCInverseTransformPoint() is called, the file is rewritten).
 *
 * Additional options to the transformer can be supplied in papszOptions.
 *
 * Options:
 *
 * <ul>
 * <li> RPC_HEIGHT: a fixed height offset to be applied to all points passed
 * in.  In this situation the Z passed into the transformation function is
 * assumed to be height above ground, and the RPC_HEIGHT is assumed to be
 * an average height above sea level for ground in the target scene.</li>
 *
 * <li> RPC_HEIGHT_SCALE: a factor used to multiply heights above ground.
 * Useful when elevation offsets of the DEM are not expressed in meters.</li>
 *
 * <li> RPC_DEM: the name of a GDAL dataset (a DEM file typically) used to
 * extract elevation offsets from. In this situation the Z passed into the
 * transformation function is assumed to be height above ground. This option
 * should be used in replacement of RPC_HEIGHT to provide a way of defining
 * a non uniform ground for the target scene</li>
 *
 * <li> RPC_DEMINTERPOLATION: the DEM interpolation ("near", "bilinear" or
 "cubic").
 *      Default is "bilinear".</li>
 *
 * <li> RPC_DEM_MISSING_VALUE: value of DEM height that must be used in case
 * the DEM has nodata value at the sampling point, or if its extent does not
 * cover the requested coordinate. When not specified, missing values will cause
 * a failed transform.</li>
 *
 * <li> RPC_DEM_SRS: (GDAL >= 3.2) WKT SRS, or any string recognized by
 * OGRSpatialReference::SetFromUserInput(), to be used as an override for DEM
 SRS.
 * Useful if DEM SRS does not have an explicit vertical component. </li>
 *
 * <li> RPC_DEM_APPLY_VDATUM_SHIFT: whether the vertical component of a compound
 * SRS for the DEM should be used (when it is present). This is useful so as to
 * be able to transform the "raw" values from the DEM expressed with respect to
 * a geoid to the heights with respect to the WGS84 ellipsoid. When this is
 * enabled, the GTIFF_REPORT_COMPD_CS configuration option will be also set
 * temporarily so as to get the vertical information from GeoTIFF
 * files. Defaults to TRUE. (GDAL >= 2.1.0)</li>
 *
 * <li> RPC_PIXEL_ERROR_THRESHOLD: overrides the dfPixErrThreshold parameter, ie
  the error (measured in pixels) allowed in the
 * iterative solution of pixel/line to lat/long computations (the other way
 * is always exact given the equations).  (GDAL >= 2.1.0)</li>
 *
 * <li> RPC_MAX_ITERATIONS: maximum number of iterations allowed in the
 * iterative solution of pixel/line to lat/long computations. Default value is
 * 10 in the absence of a DEM, or 20 if there is a DEM.  (GDAL >= 2.1.0)</li>
 *
 * <li> RPC_FOOTPRINT: WKT or GeoJSON polygon (in long / lat coordinate space)
 * with a validity footprint for the RPC. Any coordinate transformation that
 * goes from or arrive outside this footprint will be considered invalid. This
 * is useful in situations where the RPC values become highly unstable outside
 * of the area on which they have been computed for, potentially leading to
 * undesirable "echoes" / false positives. This requires GDAL to be built
 against
 * GEOS.</li>
 *
 * </ul>
 *
 * @param psRPCInfo Definition of the RPC parameters.
 *
 * @param bReversed If true "forward" transformation will be lat/long to
 * pixel/line instead of the normal pixel/line to lat/long.
 *
 * @param dfPixErrThreshold the error (measured in pixels) allowed in the
 * iterative solution of pixel/line to lat/long computations (the other way
 * is always exact given the equations). Starting with GDAL 2.1, this may also
 * be set through the RPC_PIXEL_ERROR_THRESHOLD transformer option.
 * If a negative or null value is provided, then this defaults to 0.1 pixel.
 *
 * @param papszOptions Other transformer options (i.e. RPC_HEIGHT=z).
 *
 * @return transformer callback data (deallocate with GDALDestroyTransformer()).
 */

void *GDALCreateRPCTransformerV2(const GDALRPCInfoV2 *psRPCInfo, int bReversed,
                                 double dfPixErrThreshold,
                                 CSLConstList papszOptions)

{
    /* -------------------------------------------------------------------- */
    /*      Initialize core info.                                           */
    /* -------------------------------------------------------------------- */
    GDALRPCTransformInfo *psTransform = static_cast<GDALRPCTransformInfo *>(
        CPLCalloc(sizeof(GDALRPCTransformInfo), 1));

    memcpy(&(psTransform->sRPC), psRPCInfo, sizeof(GDALRPCInfoV2));
    psTransform->bReversed = bReversed;
    const char *pszPixErrThreshold =
        CSLFetchNameValue(papszOptions, "RPC_PIXEL_ERROR_THRESHOLD");
    if (pszPixErrThreshold != nullptr)
        psTransform->dfPixErrThreshold = CPLAtof(pszPixErrThreshold);
    else if (dfPixErrThreshold > 0)
        psTransform->dfPixErrThreshold = dfPixErrThreshold;
    else
        psTransform->dfPixErrThreshold = DEFAULT_PIX_ERR_THRESHOLD;
    psTransform->dfHeightOffset = 0.0;
    psTransform->dfHeightScale = 1.0;

    memcpy(psTransform->sTI.abySignature, GDAL_GTI2_SIGNATURE,
           strlen(GDAL_GTI2_SIGNATURE));
    psTransform->sTI.pszClassName = GDAL_RPC_TRANSFORMER_CLASS_NAME;
    psTransform->sTI.pfnTransform = GDALRPCTransform;
    psTransform->sTI.pfnCleanup = GDALDestroyRPCTransformer;
    psTransform->sTI.pfnSerialize = GDALSerializeRPCTransformer;
    psTransform->sTI.pfnCreateSimilar = GDALCreateSimilarRPCTransformer;

#ifdef USE_SSE2_OPTIM
    // Make sure padfCoeffs is aligned on a 16-byte boundary for SSE2 aligned
    // loads.
    psTransform->padfCoeffs =
        psTransform->adfDoubles +
        (reinterpret_cast<GUIntptr_t>(psTransform->adfDoubles) % 16) / 8;
    memcpy(psTransform->padfCoeffs, psRPCInfo->adfLINE_NUM_COEFF,
           20 * sizeof(double));
    memcpy(psTransform->padfCoeffs + 20, psRPCInfo->adfLINE_DEN_COEFF,
           20 * sizeof(double));
    memcpy(psTransform->padfCoeffs + 40, psRPCInfo->adfSAMP_NUM_COEFF,
           20 * sizeof(double));
    memcpy(psTransform->padfCoeffs + 60, psRPCInfo->adfSAMP_DEN_COEFF,
           20 * sizeof(double));
#endif

    /* -------------------------------------------------------------------- */
    /*      Do we have a "average height" that we want to consider all      */
    /*      elevations to be relative to?                                   */
    /* -------------------------------------------------------------------- */
    const char *pszHeight = CSLFetchNameValue(papszOptions, "RPC_HEIGHT");
    if (pszHeight != nullptr)
        psTransform->dfHeightOffset = CPLAtof(pszHeight);

    /* -------------------------------------------------------------------- */
    /*                       The "height scale"                             */
    /* -------------------------------------------------------------------- */
    const char *pszHeightScale =
        CSLFetchNameValue(papszOptions, "RPC_HEIGHT_SCALE");
    if (pszHeightScale != nullptr)
        psTransform->dfHeightScale = CPLAtof(pszHeightScale);

    /* -------------------------------------------------------------------- */
    /*                       The DEM file name                              */
    /* -------------------------------------------------------------------- */
    const char *pszDEMPath = CSLFetchNameValue(papszOptions, "RPC_DEM");
    if (pszDEMPath != nullptr)
    {
        psTransform->pszDEMPath = CPLStrdup(pszDEMPath);
    }

    /* -------------------------------------------------------------------- */
    /*                      The DEM interpolation                           */
    /* -------------------------------------------------------------------- */
    const char *pszDEMInterpolation =
        CSLFetchNameValueDef(papszOptions, "RPC_DEMINTERPOLATION", "bilinear");
    if (EQUAL(pszDEMInterpolation, "near"))
    {
        psTransform->eResampleAlg = DRA_NearestNeighbour;
    }
    else if (EQUAL(pszDEMInterpolation, "bilinear"))
    {
        psTransform->eResampleAlg = DRA_Bilinear;
    }
    else if (EQUAL(pszDEMInterpolation, "cubic"))
    {
        psTransform->eResampleAlg = DRA_CubicSpline;
    }
    else
    {
        CPLDebug("RPC", "Unknown interpolation %s. Defaulting to bilinear",
                 pszDEMInterpolation);
        psTransform->eResampleAlg = DRA_Bilinear;
    }

    /* -------------------------------------------------------------------- */
    /*                       The DEM missing value                          */
    /* -------------------------------------------------------------------- */
    const char *pszDEMMissingValue =
        CSLFetchNameValue(papszOptions, "RPC_DEM_MISSING_VALUE");
    if (pszDEMMissingValue != nullptr)
    {
        psTransform->bHasDEMMissingValue = TRUE;
        psTransform->dfDEMMissingValue = CPLAtof(pszDEMMissingValue);
    }

    /* -------------------------------------------------------------------- */
    /*                        The DEM SRS override                          */
    /* -------------------------------------------------------------------- */
    const char *pszDEMSRS = CSLFetchNameValue(papszOptions, "RPC_DEM_SRS");
    if (pszDEMSRS != nullptr)
    {
        psTransform->pszDEMSRS = CPLStrdup(pszDEMSRS);
    }

    /* -------------------------------------------------------------------- */
    /*      Whether to apply vdatum shift                                   */
    /* -------------------------------------------------------------------- */
    psTransform->bApplyDEMVDatumShift =
        CPLFetchBool(papszOptions, "RPC_DEM_APPLY_VDATUM_SHIFT", true);

    psTransform->nMaxIterations =
        atoi(CSLFetchNameValueDef(papszOptions, "RPC_MAX_ITERATIONS", "0"));

    /* -------------------------------------------------------------------- */
    /*      Debug                                                           */
    /* -------------------------------------------------------------------- */
    psTransform->bRPCInverseVerbose =
        CPLTestBool(CPLGetConfigOption("RPC_INVERSE_VERBOSE", "NO"));
    const char *pszRPCInverseLog =
        CPLGetConfigOption("RPC_INVERSE_LOG", nullptr);
    if (pszRPCInverseLog != nullptr)
        psTransform->pszRPCInverseLog = CPLStrdup(pszRPCInverseLog);

    /* -------------------------------------------------------------------- */
    /*      Footprint                                                       */
    /* -------------------------------------------------------------------- */
    const char *pszFootprint = CSLFetchNameValue(papszOptions, "RPC_FOOTPRINT");
    if (pszFootprint != nullptr)
    {
        psTransform->pszRPCFootprint = CPLStrdup(pszFootprint);
        if (pszFootprint[0] == '{')
        {
            psTransform->poRPCFootprintGeom =
                OGRGeometryFactory::createFromGeoJson(pszFootprint);
        }
        else
        {
            OGRGeometryFactory::createFromWkt(
                pszFootprint, nullptr, &(psTransform->poRPCFootprintGeom));
        }
        if (psTransform->poRPCFootprintGeom)
        {
            if (OGRHasPreparedGeometrySupport())
            {
                psTransform->poRPCFootprintPreparedGeom =
                    OGRCreatePreparedGeometry(
                        OGRGeometry::ToHandle(psTransform->poRPCFootprintGeom));
            }
            else
            {
                CPLError(CE_Warning, CPLE_AppDefined,
                         "GEOS not available. RPC_FOOTPRINT will be ignored");
            }
        }
    }

    /* -------------------------------------------------------------------- */
    /*      Open DEM if needed.                                             */
    /* -------------------------------------------------------------------- */

    if (psTransform->pszDEMPath != nullptr && !GDALRPCOpenDEM(psTransform))
    {
        GDALDestroyRPCTransformer(psTransform);
        return nullptr;
    }

    /* -------------------------------------------------------------------- */
    /*      Establish a reference point for calcualating an affine          */
    /*      geotransform approximate transformation.                        */
    /* -------------------------------------------------------------------- */
    double adfGTFromLL[6] = {};
    double dfRefPixel = -1.0;
    double dfRefLine = -1.0;
    double dfRefLong = 0.0;
    double dfRefLat = 0.0;

    if (psRPCInfo->dfMIN_LONG != -180 || psRPCInfo->dfMAX_LONG != 180)
    {
        dfRefLong = (psRPCInfo->dfMIN_LONG + psRPCInfo->dfMAX_LONG) * 0.5;
        dfRefLat = (psRPCInfo->dfMIN_LAT + psRPCInfo->dfMAX_LAT) * 0.5;

        double dfX = dfRefLong;
        double dfY = dfRefLat;
        double dfZ = 0.0;
        int nSuccess = 0;
        // Try with DEM first.
        if (GDALRPCTransform(psTransform, !(psTransform->bReversed), 1, &dfX,
                             &dfY, &dfZ, &nSuccess) &&
            nSuccess)
        {
            dfRefPixel = dfX;
            dfRefLine = dfY;
        }
        else
        {
            RPCTransformPoint(psTransform, dfRefLong, dfRefLat, 0.0,
                              &dfRefPixel, &dfRefLine);
        }
    }

    // Try with scale and offset if we don't can't use bounds or
    // the results seem daft.
    if (dfRefPixel < 0.0 || dfRefLine < 0.0 || dfRefPixel > 100000 ||
        dfRefLine > 100000)
    {
        dfRefLong = psRPCInfo->dfLONG_OFF;
        dfRefLat = psRPCInfo->dfLAT_OFF;

        double dfX = dfRefLong;
        double dfY = dfRefLat;
        double dfZ = 0.0;
        int nSuccess = 0;
        // Try with DEM first.
        if (GDALRPCTransform(psTransform, !(psTransform->bReversed), 1, &dfX,
                             &dfY, &dfZ, &nSuccess) &&
            nSuccess)
        {
            dfRefPixel = dfX;
            dfRefLine = dfY;
        }
        else
        {
            RPCTransformPoint(psTransform, dfRefLong, dfRefLat, 0.0,
                              &dfRefPixel, &dfRefLine);
        }
    }

    psTransform->dfRefZ = 0.0;
    GDALRPCGetHeightAtLongLat(psTransform, dfRefLong, dfRefLat,
                              &psTransform->dfRefZ);

    /* -------------------------------------------------------------------- */
    /*      Transform nearby locations to establish affine direction        */
    /*      vectors.                                                        */
    /* -------------------------------------------------------------------- */
    double dfRefPixelDelta = 0.0;
    double dfRefLineDelta = 0.0;
    double dfLLDelta = 0.0001;

    RPCTransformPoint(psTransform, dfRefLong + dfLLDelta, dfRefLat,
                      psTransform->dfRefZ, &dfRefPixelDelta, &dfRefLineDelta);
    adfGTFromLL[1] = (dfRefPixelDelta - dfRefPixel) / dfLLDelta;
    adfGTFromLL[4] = (dfRefLineDelta - dfRefLine) / dfLLDelta;

    RPCTransformPoint(psTransform, dfRefLong, dfRefLat + dfLLDelta,
                      psTransform->dfRefZ, &dfRefPixelDelta, &dfRefLineDelta);
    adfGTFromLL[2] = (dfRefPixelDelta - dfRefPixel) / dfLLDelta;
    adfGTFromLL[5] = (dfRefLineDelta - dfRefLine) / dfLLDelta;

    adfGTFromLL[0] =
        dfRefPixel - adfGTFromLL[1] * dfRefLong - adfGTFromLL[2] * dfRefLat;
    adfGTFromLL[3] =
        dfRefLine - adfGTFromLL[4] * dfRefLong - adfGTFromLL[5] * dfRefLat;

    if (!GDALInvGeoTransform(adfGTFromLL,
                             psTransform->adfPLToLatLongGeoTransform))
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Cannot invert geotransform");
        GDALDestroyRPCTransformer(psTransform);
        return nullptr;
    }

    return psTransform;
}

/************************************************************************/
/*                 GDALDestroyReprojectionTransformer()                 */
/************************************************************************/

/** Destroy RPC transformer */
void GDALDestroyRPCTransformer(void *pTransformAlg)

{
    if (pTransformAlg == nullptr)
        return;

    GDALRPCTransformInfo *psTransform =
        static_cast<GDALRPCTransformInfo *>(pTransformAlg);

    CPLFree(psTransform->pszDEMPath);
    CPLFree(psTransform->pszDEMSRS);

    if (psTransform->poDS)
        GDALClose(psTransform->poDS);
    delete psTransform->poCacheDEM;
    if (psTransform->poCT)
        OCTDestroyCoordinateTransformation(
            reinterpret_cast<OGRCoordinateTransformationH>(psTransform->poCT));
    CPLFree(psTransform->pszRPCInverseLog);

    CPLFree(psTransform->pszRPCFootprint);
    delete psTransform->poRPCFootprintGeom;
    OGRDestroyPreparedGeometry(psTransform->poRPCFootprintPreparedGeom);

    CPLFree(pTransformAlg);
}

/************************************************************************/
/*                      RPCInverseTransformPoint()                      */
/************************************************************************/

static bool RPCInverseTransformPoint(GDALRPCTransformInfo *psTransform,
                                     double dfPixel, double dfLine,
                                     double dfUserHeight, double *pdfLong,
                                     double *pdfLat)

{
    // Memo:
    // Known to work with 40 iterations with DEM on all points (int coord and
    // +0.5,+0.5 shift) of flock1.20160216_041050_0905.tif, especially on (0,0).

    /* -------------------------------------------------------------------- */
    /*      Compute an initial approximation based on linear                */
    /*      interpolation from our reference point.                         */
    /* -------------------------------------------------------------------- */
    double dfResultX = psTransform->adfPLToLatLongGeoTransform[0] +
                       psTransform->adfPLToLatLongGeoTransform[1] * dfPixel +
                       psTransform->adfPLToLatLongGeoTransform[2] * dfLine;

    double dfResultY = psTransform->adfPLToLatLongGeoTransform[3] +
                       psTransform->adfPLToLatLongGeoTransform[4] * dfPixel +
                       psTransform->adfPLToLatLongGeoTransform[5] * dfLine;

    if (psTransform->bRPCInverseVerbose)
    {
        CPLDebug("RPC", "Computing inverse transform for (pixel,line)=(%f,%f)",
                 dfPixel, dfLine);
    }
    VSILFILE *fpLog = nullptr;
    if (psTransform->pszRPCInverseLog)
    {
        fpLog = VSIFOpenL(
            CPLResetExtensionSafe(psTransform->pszRPCInverseLog, "csvt")
                .c_str(),
            "wb");
        if (fpLog != nullptr)
        {
            VSIFPrintfL(fpLog, "Integer,Real,Real,Real,String,Real,Real\n");
            VSIFCloseL(fpLog);
        }
        fpLog = VSIFOpenL(psTransform->pszRPCInverseLog, "wb");
        if (fpLog != nullptr)
            VSIFPrintfL(
                fpLog,
                "iter,long,lat,height,WKT,error_pixel_x,error_pixel_y\n");
    }

    /* -------------------------------------------------------------------- */
    /*      Now iterate, trying to find a closer LL location that will      */
    /*      back transform to the indicated pixel and line.                 */
    /* -------------------------------------------------------------------- */
    double dfPixelDeltaX = 0.0;
    double dfPixelDeltaY = 0.0;
    double dfLastResultX = 0.0;
    double dfLastResultY = 0.0;
    double dfLastPixelDeltaX = 0.0;
    double dfLastPixelDeltaY = 0.0;
    bool bLastPixelDeltaValid = false;
    const int nMaxIterations = (psTransform->nMaxIterations > 0)
                                   ? psTransform->nMaxIterations
                               : (psTransform->poDS != nullptr) ? 20
                                                                : 10;
    int nCountConsecutiveErrorBelow2 = 0;

    int iIter = 0;  // Used after for.
    for (; iIter < nMaxIterations; iIter++)
    {
        double dfBackPixel = 0.0;
        double dfBackLine = 0.0;

        // Update DEMH.
        double dfDEMH = 0.0;
        double dfDEMPixel = 0.0;
        double dfDEMLine = 0.0;
        if (!GDALRPCGetHeightAtLongLat(psTransform, dfResultX, dfResultY,
                                       &dfDEMH, &dfDEMPixel, &dfDEMLine))
        {
            if (psTransform->poDS)
            {
                CPLDebug("RPC", "DEM (pixel, line) = (%g, %g)", dfDEMPixel,
                         dfDEMLine);
            }

            // The first time, the guess might be completely out of the
            // validity of the DEM, so pickup the "reference Z" as the
            // first guess or the closest point of the DEM by snapping to it.
            if (iIter == 0)
            {
                bool bUseRefZ = true;
                if (psTransform->poDS)
                {
                    if (dfDEMPixel >= psTransform->poDS->GetRasterXSize())
                        dfDEMPixel = psTransform->poDS->GetRasterXSize() - 0.5;
                    else if (dfDEMPixel < 0)
                        dfDEMPixel = 0.5;
                    if (dfDEMLine >= psTransform->poDS->GetRasterYSize())
                        dfDEMLine = psTransform->poDS->GetRasterYSize() - 0.5;
                    else if (dfDEMPixel < 0)
                        dfDEMPixel = 0.5;
                    if (GDALRPCGetDEMHeight(psTransform, dfDEMPixel, dfDEMLine,
                                            &dfDEMH))
                    {
                        bUseRefZ = false;
                        CPLDebug("RPC",
                                 "Iteration %d for (pixel, line) = (%g, %g): "
                                 "No elevation value at %.15g %.15g. "
                                 "Using elevation %g at DEM (pixel, line) = "
                                 "(%g, %g) (snapping to boundaries) instead",
                                 iIter, dfPixel, dfLine, dfResultX, dfResultY,
                                 dfDEMH, dfDEMPixel, dfDEMLine);
                    }
                }
                if (bUseRefZ)
                {
                    dfDEMH = psTransform->dfRefZ;
                    CPLDebug("RPC",
                             "Iteration %d for (pixel, line) = (%g, %g): "
                             "No elevation value at %.15g %.15g. "
                             "Using elevation %g of reference point instead",
                             iIter, dfPixel, dfLine, dfResultX, dfResultY,
                             dfDEMH);
                }
            }
            else
            {
                CPLDebug("RPC",
                         "Iteration %d for (pixel, line) = (%g, %g): "
                         "No elevation value at %.15g %.15g. Erroring out",
                         iIter, dfPixel, dfLine, dfResultX, dfResultY);
                if (fpLog)
                    VSIFCloseL(fpLog);
                return false;
            }
        }

        RPCTransformPoint(psTransform, dfResultX, dfResultY,
                          dfUserHeight + dfDEMH, &dfBackPixel, &dfBackLine);

        dfPixelDeltaX = dfBackPixel - dfPixel;
        dfPixelDeltaY = dfBackLine - dfLine;

        if (psTransform->bRPCInverseVerbose)
        {
            CPLDebug("RPC",
                     "Iter %d: dfPixelDeltaX=%.02f, dfPixelDeltaY=%.02f, "
                     "long=%f, lat=%f, height=%f",
                     iIter, dfPixelDeltaX, dfPixelDeltaY, dfResultX, dfResultY,
                     dfUserHeight + dfDEMH);
        }
        if (fpLog != nullptr)
        {
            VSIFPrintfL(fpLog,
                        "%d,%.12f,%.12f,%f,\"POINT(%.12f %.12f)\",%f,%f\n",
                        iIter, dfResultX, dfResultY, dfUserHeight + dfDEMH,
                        dfResultX, dfResultY, dfPixelDeltaX, dfPixelDeltaY);
        }

        const double dfError =
            std::max(std::abs(dfPixelDeltaX), std::abs(dfPixelDeltaY));
        if (dfError < psTransform->dfPixErrThreshold)
        {
            iIter = -1;
            if (psTransform->bRPCInverseVerbose)
            {
                CPLDebug("RPC", "Converged!");
            }
            break;
        }
        else if (psTransform->poDS != nullptr && bLastPixelDeltaValid &&
                 dfPixelDeltaX * dfLastPixelDeltaX < 0 &&
                 dfPixelDeltaY * dfLastPixelDeltaY < 0)
        {
            // When there is a DEM, if the error changes sign, we might
            // oscillate forever, so take a mean position as a new guess.
            if (psTransform->bRPCInverseVerbose)
            {
                CPLDebug("RPC",
                         "Oscillation detected. "
                         "Taking mean of 2 previous results as new guess");
            }
            dfResultX = (fabs(dfPixelDeltaX) * dfLastResultX +
                         fabs(dfLastPixelDeltaX) * dfResultX) /
                        (fabs(dfPixelDeltaX) + fabs(dfLastPixelDeltaX));
            dfResultY = (fabs(dfPixelDeltaY) * dfLastResultY +
                         fabs(dfLastPixelDeltaY) * dfResultY) /
                        (fabs(dfPixelDeltaY) + fabs(dfLastPixelDeltaY));
            bLastPixelDeltaValid = false;
            nCountConsecutiveErrorBelow2 = 0;
            continue;
        }

        double dfBoostFactor = 1.0;
        if (psTransform->poDS != nullptr && nCountConsecutiveErrorBelow2 >= 5 &&
            dfError < 2)
        {
            // When there is a DEM, if we remain below a given threshold
            // (somewhat arbitrarily set to 2 pixels) for some time, apply a
            // "boost factor" for the new guessed result, in the hope we will go
            // out of the somewhat current stuck situation.
            dfBoostFactor = 10;
            if (psTransform->bRPCInverseVerbose)
            {
                CPLDebug("RPC", "Applying boost factor 10");
            }
        }

        if (dfError < 2)
            nCountConsecutiveErrorBelow2++;
        else
            nCountConsecutiveErrorBelow2 = 0;

        const double dfNewResultX =
            dfResultX -
            (dfPixelDeltaX * psTransform->adfPLToLatLongGeoTransform[1] *
             dfBoostFactor) -
            (dfPixelDeltaY * psTransform->adfPLToLatLongGeoTransform[2] *
             dfBoostFactor);
        const double dfNewResultY =
            dfResultY -
            (dfPixelDeltaX * psTransform->adfPLToLatLongGeoTransform[4] *
             dfBoostFactor) -
            (dfPixelDeltaY * psTransform->adfPLToLatLongGeoTransform[5] *
             dfBoostFactor);

        dfLastResultX = dfResultX;
        dfLastResultY = dfResultY;
        dfResultX = dfNewResultX;
        dfResultY = dfNewResultY;
        dfLastPixelDeltaX = dfPixelDeltaX;
        dfLastPixelDeltaY = dfPixelDeltaY;
        bLastPixelDeltaValid = true;
    }
    if (fpLog != nullptr)
        VSIFCloseL(fpLog);

    if (iIter != -1)
    {
        CPLDebug("RPC", "Failed Iterations %d: Got: %.16g,%.16g  Offset=%g,%g",
                 iIter, dfResultX, dfResultY, dfPixelDeltaX, dfPixelDeltaY);
        return false;
    }

    *pdfLong = dfResultX;
    *pdfLat = dfResultY;
    return true;
}

/************************************************************************/
/*                        GDALRPCGetDEMHeight()                         */
/************************************************************************/

static int GDALRPCGetDEMHeight(GDALRPCTransformInfo *psTransform,
                               const double dfXIn, const double dfYIn,
                               double *pdfDEMH)
{
    GDALRIOResampleAlg eResample = GDALRIOResampleAlg::GRIORA_NearestNeighbour;
    switch (psTransform->eResampleAlg)
    {
        case DEMResampleAlg::DRA_NearestNeighbour:
            eResample = GDALRIOResampleAlg::GRIORA_NearestNeighbour;
            break;
        case DEMResampleAlg::DRA_Bilinear:
            eResample = GDALRIOResampleAlg::GRIORA_Bilinear;
            break;
        case DEMResampleAlg::DRA_CubicSpline:
            eResample = GDALRIOResampleAlg::GRIORA_CubicSpline;
            break;
    }

    std::unique_ptr<DoublePointsCache> cacheDEM{psTransform->poCacheDEM};
    int res =
        GDALInterpolateAtPoint(psTransform->poDS->GetRasterBand(1), eResample,
                               cacheDEM, dfXIn, dfYIn, pdfDEMH, nullptr);
    psTransform->poCacheDEM = cacheDEM.release();
    return res;
}

/************************************************************************/
/*                           RPCIsValidLongLat()                        */
/************************************************************************/

static bool RPCIsValidLongLat(const GDALRPCTransformInfo *psTransform,
                              double dfLong, double dfLat)
{
    if (!psTransform->poRPCFootprintPreparedGeom)
        return true;

    OGRPoint p(dfLong, dfLat);
    return CPL_TO_BOOL(OGRPreparedGeometryContains(
        psTransform->poRPCFootprintPreparedGeom, OGRGeometry::ToHandle(&p)));
}

/************************************************************************/
/*                    GDALRPCTransformWholeLineWithDEM()                */
/************************************************************************/

static int
GDALRPCTransformWholeLineWithDEM(const GDALRPCTransformInfo *psTransform,
                                 int nPointCount, double *padfX, double *padfY,
                                 double *padfZ, int *panSuccess, int nXLeft,
                                 int nXWidth, int nYTop, int nYHeight)
{
    double *padfDEMBuffer = static_cast<double *>(
        VSI_MALLOC3_VERBOSE(sizeof(double), nXWidth, nYHeight));
    if (padfDEMBuffer == nullptr)
    {
        for (int i = 0; i < nPointCount; i++)
            panSuccess[i] = FALSE;
        return FALSE;
    }
    CPLErr eErr = psTransform->poDS->GetRasterBand(1)->RasterIO(
        GF_Read, nXLeft, nYTop, nXWidth, nYHeight, padfDEMBuffer, nXWidth,
        nYHeight, GDT_Float64, 0, 0, nullptr);
    if (eErr != CE_None)
    {
        for (int i = 0; i < nPointCount; i++)
            panSuccess[i] = FALSE;
        VSIFree(padfDEMBuffer);
        return FALSE;
    }

    int bGotNoDataValue = FALSE;
    const double dfNoDataValue =
        psTransform->poDS->GetRasterBand(1)->GetNoDataValue(&bGotNoDataValue);

    // dfY in pixel center convention.
    const double dfY = psTransform->adfDEMReverseGeoTransform[3] +
                       padfY[0] * psTransform->adfDEMReverseGeoTransform[5] -
                       0.5;
    const int nY = static_cast<int>(dfY);
    const double dfDeltaY = dfY - nY;

    int bRet = TRUE;
    for (int i = 0; i < nPointCount; i++)
    {
        if (padfX[i] == HUGE_VAL)
        {
            bRet = FALSE;
            panSuccess[i] = FALSE;
            continue;
        }

        double dfDEMH = 0.0;
        const double dfZ_i = padfZ ? padfZ[i] : 0.0;

        if (psTransform->eResampleAlg == DRA_CubicSpline)
        {
            // dfX in pixel center convention.
            const double dfX =
                psTransform->adfDEMReverseGeoTransform[0] +
                padfX[i] * psTransform->adfDEMReverseGeoTransform[1] - 0.5;
            const int nX = static_cast<int>(dfX);
            const double dfDeltaX = dfX - nX;

            const int nXNew = nX - 1;

            double dfSumH = 0.0;
            double dfSumWeight = 0.0;
            for (int k_i = 0; k_i < 4; k_i++)
            {
                // Loop across the X axis.
                for (int k_j = 0; k_j < 4; k_j++)
                {
                    // Calculate the weight for the specified pixel according
                    // to the bicubic b-spline kernel we're using for
                    // interpolation.
                    const int dKernIndX = k_j - 1;
                    const int dKernIndY = k_i - 1;
                    const double dfPixelWeight =
                        CubicSplineKernel(dKernIndX - dfDeltaX) *
                        CubicSplineKernel(dKernIndY - dfDeltaY);

                    // Create a sum of all values
                    // adjusted for the pixel's calculated weight.
                    const double dfElev =
                        padfDEMBuffer[k_i * nXWidth + nXNew - nXLeft + k_j];
                    if (bGotNoDataValue &&
                        ARE_REAL_EQUAL(dfNoDataValue, dfElev))
                        continue;

                    dfSumH += dfElev * dfPixelWeight;
                    dfSumWeight += dfPixelWeight;
                }
            }
            if (dfSumWeight == 0.0)
            {
                if (psTransform->bHasDEMMissingValue)
                    dfDEMH = psTransform->dfDEMMissingValue;
                else
                {
                    bRet = FALSE;
                    panSuccess[i] = FALSE;
                    continue;
                }
            }
            else
                dfDEMH = dfSumH / dfSumWeight;
        }
        else if (psTransform->eResampleAlg == DRA_Bilinear)
        {
            // dfX in pixel center convention.
            const double dfX =
                psTransform->adfDEMReverseGeoTransform[0] +
                padfX[i] * psTransform->adfDEMReverseGeoTransform[1] - 0.5;
            const int nX = static_cast<int>(dfX);
            const double dfDeltaX = dfX - nX;

            // Bilinear interpolation.
            double adfElevData[4] = {};
            memcpy(adfElevData, padfDEMBuffer + nX - nXLeft,
                   2 * sizeof(double));
            memcpy(adfElevData + 2, padfDEMBuffer + nXWidth + nX - nXLeft,
                   2 * sizeof(double));

            int bFoundNoDataElev = FALSE;
            if (bGotNoDataValue)
            {
                int k_valid_sample = -1;
                for (int k_i = 0; k_i < 4; k_i++)
                {
                    if (ARE_REAL_EQUAL(dfNoDataValue, adfElevData[k_i]))
                    {
                        bFoundNoDataElev = TRUE;
                    }
                    else if (k_valid_sample < 0)
                    {
                        k_valid_sample = k_i;
                    }
                }
                if (bFoundNoDataElev)
                {
                    if (k_valid_sample >= 0)
                    {
                        if (!RPCIsValidLongLat(psTransform, padfX[i], padfY[i]))
                        {
                            bRet = FALSE;
                            panSuccess[i] = FALSE;
                            padfX[i] = HUGE_VAL;
                            padfY[i] = HUGE_VAL;
                            continue;
                        }
                        dfDEMH = adfElevData[k_valid_sample];
                        RPCTransformPoint(
                            psTransform, padfX[i], padfY[i],
                            dfZ_i + (psTransform->dfHeightOffset + dfDEMH) *
                                        psTransform->dfHeightScale,
                            padfX + i, padfY + i);

                        panSuccess[i] = TRUE;
                        continue;
                    }
                    else if (psTransform->bHasDEMMissingValue)
                    {
                        if (!RPCIsValidLongLat(psTransform, padfX[i], padfY[i]))
                        {
                            bRet = FALSE;
                            panSuccess[i] = FALSE;
                            padfX[i] = HUGE_VAL;
                            padfY[i] = HUGE_VAL;
                            continue;
                        }
                        dfDEMH = psTransform->dfDEMMissingValue;
                        RPCTransformPoint(
                            psTransform, padfX[i], padfY[i],
                            dfZ_i + (psTransform->dfHeightOffset + dfDEMH) *
                                        psTransform->dfHeightScale,
                            padfX + i, padfY + i);

                        panSuccess[i] = TRUE;
                        continue;
                    }
                    else
                    {
                        bRet = FALSE;
                        panSuccess[i] = FALSE;
                        padfX[i] = HUGE_VAL;
                        padfY[i] = HUGE_VAL;
                        continue;
                    }
                }
            }
            const double dfDeltaX1 = 1.0 - dfDeltaX;
            const double dfDeltaY1 = 1.0 - dfDeltaY;

            const double dfXZ1 =
                adfElevData[0] * dfDeltaX1 + adfElevData[1] * dfDeltaX;
            const double dfXZ2 =
                adfElevData[2] * dfDeltaX1 + adfElevData[3] * dfDeltaX;
            const double dfYZ = dfXZ1 * dfDeltaY1 + dfXZ2 * dfDeltaY;
            dfDEMH = dfYZ;
        }
        else
        {
            const double dfX =
                psTransform->adfDEMReverseGeoTransform[0] +
                padfX[i] * psTransform->adfDEMReverseGeoTransform[1];
            const int nX = int(dfX);

            dfDEMH = padfDEMBuffer[nX - nXLeft];
            if (bGotNoDataValue && ARE_REAL_EQUAL(dfNoDataValue, dfDEMH))
            {
                if (psTransform->bHasDEMMissingValue)
                    dfDEMH = psTransform->dfDEMMissingValue;
                else
                {
                    bRet = FALSE;
                    panSuccess[i] = FALSE;
                    padfX[i] = HUGE_VAL;
                    padfY[i] = HUGE_VAL;
                    continue;
                }
            }
        }

        if (!RPCIsValidLongLat(psTransform, padfX[i], padfY[i]))
        {
            bRet = FALSE;
            panSuccess[i] = FALSE;
            padfX[i] = HUGE_VAL;
            padfY[i] = HUGE_VAL;
            continue;
        }
        RPCTransformPoint(psTransform, padfX[i], padfY[i],
                          dfZ_i + (psTransform->dfHeightOffset + dfDEMH) *
                                      psTransform->dfHeightScale,
                          padfX + i, padfY + i);

        panSuccess[i] = TRUE;
    }

    VSIFree(padfDEMBuffer);

    return bRet;
}

/************************************************************************/
/*                           GDALRPCOpenDEM()                           */
/************************************************************************/

static bool GDALRPCOpenDEM(GDALRPCTransformInfo *psTransform)
{
    CPLAssert(psTransform->pszDEMPath != nullptr);

    bool bIsValid = false;

    CPLString osPrevValueConfigOption;
    if (psTransform->bApplyDEMVDatumShift)
    {
        osPrevValueConfigOption =
            CPLGetThreadLocalConfigOption("GTIFF_REPORT_COMPD_CS", "");
        CPLSetThreadLocalConfigOption("GTIFF_REPORT_COMPD_CS", "YES");
    }
    CPLConfigOptionSetter oSetter("CPL_ALLOW_VSISTDIN", "NO", true);
    psTransform->poDS =
        GDALDataset::FromHandle(GDALOpen(psTransform->pszDEMPath, GA_ReadOnly));
    if (psTransform->poDS != nullptr &&
        psTransform->poDS->GetRasterCount() >= 1)
    {
        OGRSpatialReference oDEMSRS;
        if (psTransform->pszDEMSRS != nullptr)
        {
            oDEMSRS.SetFromUserInput(psTransform->pszDEMSRS);
            oDEMSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        }

        auto poDSSpaRefSrc = psTransform->pszDEMSRS != nullptr
                                 ? &oDEMSRS
                                 : psTransform->poDS->GetSpatialRef();
        if (poDSSpaRefSrc)
        {
            auto poDSSpaRef = poDSSpaRefSrc->Clone();

            if (!psTransform->bApplyDEMVDatumShift)
                poDSSpaRef->StripVertical();

            auto wkt_EPSG_4979 =
                "GEODCRS[\"WGS 84\",\n"
                "    DATUM[\"World Geodetic System 1984\",\n"
                "        ELLIPSOID[\"WGS 84\",6378137,298.257223563,\n"
                "            LENGTHUNIT[\"metre\",1]]],\n"
                "    PRIMEM[\"Greenwich\",0,\n"
                "        ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
                "    CS[ellipsoidal,3],\n"
                "        AXIS[\"geodetic latitude (Lat)\",north,\n"
                "            ORDER[1],\n"
                "            ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
                "        AXIS[\"geodetic longitude (Lon)\",east,\n"
                "            ORDER[2],\n"
                "            ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
                "        AXIS[\"ellipsoidal height (h)\",up,\n"
                "            ORDER[3],\n"
                "            LENGTHUNIT[\"metre\",1]],\n"
                "    AREA[\"World (by country)\"],\n"
                "    BBOX[-90,-180,90,180],\n"
                "    ID[\"EPSG\",4979]]";
            OGRSpatialReference *poWGSSpaRef = new OGRSpatialReference(
                poDSSpaRef->IsCompound() ? wkt_EPSG_4979
                                         : SRS_WKT_WGS84_LAT_LONG);
            poWGSSpaRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);

            if (!poWGSSpaRef->IsSame(poDSSpaRef))
                psTransform->poCT =
                    OGRCreateCoordinateTransformation(poWGSSpaRef, poDSSpaRef);

            if (psTransform->poCT != nullptr && !poDSSpaRef->IsCompound())
            {
                // Empiric attempt to guess if the coordinate transformation
                // to WGS84 is a no-op. For example for NED13 datasets in
                // NAD83.
                double adfX[] = {-179.0, 179.0, 179.0, -179.0, 0.0, 0.0};
                double adfY[] = {89.0, 89.0, -89.0, -89.0, 0.0, 0.0};
                double adfZ[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

                // Also test with a "reference point" from the RPC values.
                double dfRefLong = 0.0;
                double dfRefLat = 0.0;
                if (psTransform->sRPC.dfMIN_LONG != -180 ||
                    psTransform->sRPC.dfMAX_LONG != 180)
                {
                    dfRefLong = (psTransform->sRPC.dfMIN_LONG +
                                 psTransform->sRPC.dfMAX_LONG) *
                                0.5;
                    dfRefLat = (psTransform->sRPC.dfMIN_LAT +
                                psTransform->sRPC.dfMAX_LAT) *
                               0.5;
                }
                else
                {
                    dfRefLong = psTransform->sRPC.dfLONG_OFF;
                    dfRefLat = psTransform->sRPC.dfLAT_OFF;
                }
                adfX[5] = dfRefLong;
                adfY[5] = dfRefLat;

                if (psTransform->poCT->Transform(6, adfX, adfY, adfZ) &&
                    fabs(adfX[0] - -179.0) < 1.0e-12 &&
                    fabs(adfY[0] - 89.0) < 1.0e-12 &&
                    fabs(adfX[1] - 179.0) < 1.0e-12 &&
                    fabs(adfY[1] - 89.0) < 1.0e-12 &&
                    fabs(adfX[2] - 179.0) < 1.0e-12 &&
                    fabs(adfY[2] - -89.0) < 1.0e-12 &&
                    fabs(adfX[3] - -179.0) < 1.0e-12 &&
                    fabs(adfY[3] - -89.0) < 1.0e-12 &&
                    fabs(adfX[4] - 0.0) < 1.0e-12 &&
                    fabs(adfY[4] - 0.0) < 1.0e-12 &&
                    fabs(adfX[5] - dfRefLong) < 1.0e-12 &&
                    fabs(adfY[5] - dfRefLat) < 1.0e-12)
                {
                    CPLDebug("RPC",
                             "Short-circuiting coordinate transformation "
                             "from DEM SRS to WGS 84 due to apparent nop");
                    delete psTransform->poCT;
                    psTransform->poCT = nullptr;
                }
            }

            delete poWGSSpaRef;
            delete poDSSpaRef;
        }

        if (psTransform->poDS->GetGeoTransform(
                *reinterpret_cast<GDALGeoTransform *>(
                    psTransform->adfDEMGeoTransform)) == CE_None &&
            GDALInvGeoTransform(psTransform->adfDEMGeoTransform,
                                psTransform->adfDEMReverseGeoTransform))
        {
            bIsValid = true;
        }
    }

    if (psTransform->bApplyDEMVDatumShift)
    {
        CPLSetThreadLocalConfigOption("GTIFF_REPORT_COMPD_CS",
                                      !osPrevValueConfigOption.empty()
                                          ? osPrevValueConfigOption.c_str()
                                          : nullptr);
    }

    return bIsValid;
}

/************************************************************************/
/*                          GDALRPCTransform()                          */
/************************************************************************/

/** RPC transform */
int GDALRPCTransform(void *pTransformArg, int bDstToSrc, int nPointCount,
                     double *padfX, double *padfY, double *padfZ,
                     int *panSuccess)

{
    VALIDATE_POINTER1(pTransformArg, "GDALRPCTransform", 0);

    GDALRPCTransformInfo *psTransform =
        static_cast<GDALRPCTransformInfo *>(pTransformArg);

    if (psTransform->bReversed)
        bDstToSrc = !bDstToSrc;

    /* -------------------------------------------------------------------- */
    /*      The simple case is transforming from lat/long to pixel/line.    */
    /*      Just apply the equations directly.                              */
    /* -------------------------------------------------------------------- */
    if (bDstToSrc)
    {
        // Optimization to avoid doing too many picking in DEM in the particular
        // case where each point to transform is on a single line of the DEM.
        // To make it simple and fast we check that all input latitudes are
        // identical, that the DEM is in WGS84 geodetic and that it has no
        // rotation.  Such case is for example triggered when doing gdalwarp
        // with a target SRS of EPSG:4326 or EPSG:3857.
        if (nPointCount >= 10 && psTransform->poDS != nullptr &&
            psTransform->poCT == nullptr &&
            padfY[0] == padfY[nPointCount - 1] &&
            padfY[0] == padfY[nPointCount / 2] &&
            psTransform->adfDEMReverseGeoTransform[1] > 0.0 &&
            psTransform->adfDEMReverseGeoTransform[2] == 0.0 &&
            psTransform->adfDEMReverseGeoTransform[4] == 0.0 &&
            CPLTestBool(CPLGetConfigOption("GDAL_RPC_DEM_OPTIM", "YES")))
        {
            bool bUseOptimized = true;
            double dfMinX = padfX[0];
            double dfMaxX = padfX[0];
            for (int i = 1; i < nPointCount; i++)
            {
                if (padfY[i] != padfY[0])
                {
                    bUseOptimized = false;
                    break;
                }
                if (padfX[i] < dfMinX)
                    dfMinX = padfX[i];
                if (padfX[i] > dfMaxX)
                    dfMaxX = padfX[i];
            }
            if (bUseOptimized)
            {
                double dfX1 = 0.0;
                double dfY1 = 0.0;
                double dfX2 = 0.0;
                double dfY2 = 0.0;
                GDALApplyGeoTransform(psTransform->adfDEMReverseGeoTransform,
                                      dfMinX, padfY[0], &dfX1, &dfY1);
                GDALApplyGeoTransform(psTransform->adfDEMReverseGeoTransform,
                                      dfMaxX, padfY[0], &dfX2, &dfY2);

                // Convert to center of pixel convention for reading the image
                // data.
                if (psTransform->eResampleAlg != DRA_NearestNeighbour)
                {
                    dfX1 -= 0.5;
                    dfY1 -= 0.5;
                    dfX2 -= 0.5;
                    // dfY2 -= 0.5;
                }
                int nXLeft = static_cast<int>(floor(dfX1));
                int nXRight = static_cast<int>(floor(dfX2));
                int nXWidth = nXRight - nXLeft + 1;
                int nYTop = static_cast<int>(floor(dfY1));
                int nYHeight;
                if (psTransform->eResampleAlg == DRA_CubicSpline)
                {
                    nXLeft--;
                    nXWidth += 3;
                    nYTop--;
                    nYHeight = 4;
                }
                else if (psTransform->eResampleAlg == DRA_Bilinear)
                {
                    nXWidth++;
                    nYHeight = 2;
                }
                else
                {
                    nYHeight = 1;
                }
                if (nXLeft >= 0 &&
                    nXLeft + nXWidth <= psTransform->poDS->GetRasterXSize() &&
                    nYTop >= 0 &&
                    nYTop + nYHeight <= psTransform->poDS->GetRasterYSize())
                {
                    static bool bOnce = false;
                    if (!bOnce)
                    {
                        bOnce = true;
                        CPLDebug("RPC",
                                 "Using GDALRPCTransformWholeLineWithDEM");
                    }
                    return GDALRPCTransformWholeLineWithDEM(
                        psTransform, nPointCount, padfX, padfY, padfZ,
                        panSuccess, nXLeft, nXWidth, nYTop, nYHeight);
                }
            }
        }

        int bRet = TRUE;
        for (int i = 0; i < nPointCount; i++)
        {
            if (!RPCIsValidLongLat(psTransform, padfX[i], padfY[i]))
            {
                bRet = FALSE;
                panSuccess[i] = FALSE;
                padfX[i] = HUGE_VAL;
                padfY[i] = HUGE_VAL;
                continue;
            }
            double dfHeight = 0.0;
            if (!GDALRPCGetHeightAtLongLat(psTransform, padfX[i], padfY[i],
                                           &dfHeight))
            {
                bRet = FALSE;
                panSuccess[i] = FALSE;
                padfX[i] = HUGE_VAL;
                padfY[i] = HUGE_VAL;
                continue;
            }

            RPCTransformPoint(psTransform, padfX[i], padfY[i],
                              (padfZ ? padfZ[i] : 0.0) + dfHeight, padfX + i,
                              padfY + i);
            panSuccess[i] = TRUE;
        }

        return bRet;
    }

    if (padfZ == nullptr)
    {
        CPLError(CE_Failure, CPLE_NotSupported,
                 "Z array should be provided for reverse RPC computation");
        for (int i = 0; i < nPointCount; i++)
            panSuccess[i] = FALSE;
        return FALSE;
    }

    /* -------------------------------------------------------------------- */
    /*      Compute the inverse (pixel/line/height to lat/long).  This      */
    /*      function uses an iterative method from an initial linear        */
    /*      approximation.                                                  */
    /* -------------------------------------------------------------------- */
    int bRet = TRUE;
    for (int i = 0; i < nPointCount; i++)
    {
        double dfResultX = 0.0;
        double dfResultY = 0.0;

        if (!RPCInverseTransformPoint(psTransform, padfX[i], padfY[i], padfZ[i],
                                      &dfResultX, &dfResultY))
        {
            bRet = FALSE;
            panSuccess[i] = FALSE;
            padfX[i] = HUGE_VAL;
            padfY[i] = HUGE_VAL;
            continue;
        }
        if (!RPCIsValidLongLat(psTransform, padfX[i], padfY[i]))
        {
            bRet = FALSE;
            panSuccess[i] = FALSE;
            padfX[i] = HUGE_VAL;
            padfY[i] = HUGE_VAL;
            continue;
        }

        padfX[i] = dfResultX;
        padfY[i] = dfResultY;

        panSuccess[i] = TRUE;
    }

    return bRet;
}

/************************************************************************/
/*                    GDALSerializeRPCTransformer()                     */
/************************************************************************/

CPLXMLNode *GDALSerializeRPCTransformer(void *pTransformArg)

{
    VALIDATE_POINTER1(pTransformArg, "GDALSerializeRPCTransformer", nullptr);

    GDALRPCTransformInfo *psInfo =
        static_cast<GDALRPCTransformInfo *>(pTransformArg);

    CPLXMLNode *psTree =
        CPLCreateXMLNode(nullptr, CXT_Element, "RPCTransformer");

    /* -------------------------------------------------------------------- */
    /*      Serialize bReversed.                                            */
    /* -------------------------------------------------------------------- */
    CPLCreateXMLElementAndValue(psTree, "Reversed",
                                CPLString().Printf("%d", psInfo->bReversed));

    /* -------------------------------------------------------------------- */
    /*      Serialize Height Offset.                                        */
    /* -------------------------------------------------------------------- */
    CPLCreateXMLElementAndValue(
        psTree, "HeightOffset",
        CPLString().Printf("%.15g", psInfo->dfHeightOffset));

    /* -------------------------------------------------------------------- */
    /*      Serialize Height Scale.                                         */
    /* -------------------------------------------------------------------- */
    if (psInfo->dfHeightScale != 1.0)
        CPLCreateXMLElementAndValue(
            psTree, "HeightScale",
            CPLString().Printf("%.15g", psInfo->dfHeightScale));

    /* -------------------------------------------------------------------- */
    /*      Serialize DEM path.                                             */
    /* -------------------------------------------------------------------- */
    if (psInfo->pszDEMPath != nullptr)
    {
        CPLCreateXMLElementAndValue(
            psTree, "DEMPath", CPLString().Printf("%s", psInfo->pszDEMPath));

        /* --------------------------------------------------------------------
         */
        /*      Serialize DEM interpolation */
        /* --------------------------------------------------------------------
         */
        CPLCreateXMLElementAndValue(
            psTree, "DEMInterpolation",
            GDALSerializeRPCDEMResample(psInfo->eResampleAlg));

        if (psInfo->bHasDEMMissingValue)
        {
            CPLCreateXMLElementAndValue(
                psTree, "DEMMissingValue",
                CPLSPrintf("%.17g", psInfo->dfDEMMissingValue));
        }

        CPLCreateXMLElementAndValue(psTree, "DEMApplyVDatumShift",
                                    psInfo->bApplyDEMVDatumShift ? "true"
                                                                 : "false");

        /* --------------------------------------------------------------------
         */
        /*      Serialize DEM SRS */
        /* --------------------------------------------------------------------
         */
        if (psInfo->pszDEMSRS != nullptr)
        {
            CPLCreateXMLElementAndValue(psTree, "DEMSRS", psInfo->pszDEMSRS);
        }
    }

    /* -------------------------------------------------------------------- */
    /*      Serialize pixel error threshold.                                */
    /* -------------------------------------------------------------------- */
    CPLCreateXMLElementAndValue(
        psTree, "PixErrThreshold",
        CPLString().Printf("%.15g", psInfo->dfPixErrThreshold));

    /* -------------------------------------------------------------------- */
    /*      RPC metadata.                                                   */
    /* -------------------------------------------------------------------- */
    char **papszMD = RPCInfoV2ToMD(&(psInfo->sRPC));
    CPLXMLNode *psMD = CPLCreateXMLNode(psTree, CXT_Element, "Metadata");

    for (int i = 0; papszMD != nullptr && papszMD[i] != nullptr; i++)
    {
        char *pszKey = nullptr;

        const char *pszRawValue = CPLParseNameValue(papszMD[i], &pszKey);

        CPLXMLNode *psMDI = CPLCreateXMLNode(psMD, CXT_Element, "MDI");
        CPLSetXMLValue(psMDI, "#key", pszKey);
        CPLCreateXMLNode(psMDI, CXT_Text, pszRawValue);

        CPLFree(pszKey);
    }

    CSLDestroy(papszMD);

    return psTree;
}

/************************************************************************/
/*                   GDALDeserializeRPCTransformer()                    */
/************************************************************************/

void *GDALDeserializeRPCTransformer(CPLXMLNode *psTree)

{
    char **papszOptions = nullptr;

    /* -------------------------------------------------------------------- */
    /*      Collect metadata.                                               */
    /* -------------------------------------------------------------------- */
    CPLXMLNode *psMetadata = CPLGetXMLNode(psTree, "Metadata");

    if (psMetadata == nullptr || psMetadata->eType != CXT_Element ||
        !EQUAL(psMetadata->pszValue, "Metadata"))
        return nullptr;

    char **papszMD = nullptr;
    for (CPLXMLNode *psMDI = psMetadata->psChild; psMDI != nullptr;
         psMDI = psMDI->psNext)
    {
        if (!EQUAL(psMDI->pszValue, "MDI") || psMDI->eType != CXT_Element ||
            psMDI->psChild == nullptr || psMDI->psChild->psNext == nullptr ||
            psMDI->psChild->eType != CXT_Attribute ||
            psMDI->psChild->psChild == nullptr)
            continue;

        papszMD = CSLSetNameValue(papszMD, psMDI->psChild->psChild->pszValue,
                                  psMDI->psChild->psNext->pszValue);
    }

    GDALRPCInfoV2 sRPC;
    if (!GDALExtractRPCInfoV2(papszMD, &sRPC))
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "Failed to reconstitute RPC transformer.");
        CSLDestroy(papszMD);
        return nullptr;
    }

    CSLDestroy(papszMD);

    /* -------------------------------------------------------------------- */
    /*      Get other flags.                                                */
    /* -------------------------------------------------------------------- */
    const int bReversed = atoi(CPLGetXMLValue(psTree, "Reversed", "0"));

    const double dfPixErrThreshold =
        CPLAtof(CPLGetXMLValue(psTree, "PixErrThreshold",
                               CPLSPrintf("%f", DEFAULT_PIX_ERR_THRESHOLD)));

    papszOptions = CSLSetNameValue(papszOptions, "RPC_HEIGHT",
                                   CPLGetXMLValue(psTree, "HeightOffset", "0"));
    papszOptions = CSLSetNameValue(papszOptions, "RPC_HEIGHT_SCALE",
                                   CPLGetXMLValue(psTree, "HeightScale", "1"));
    const char *pszDEMPath = CPLGetXMLValue(psTree, "DEMPath", nullptr);
    if (pszDEMPath != nullptr)
        papszOptions = CSLSetNameValue(papszOptions, "RPC_DEM", pszDEMPath);

    const char *pszDEMInterpolation =
        CPLGetXMLValue(psTree, "DEMInterpolation", "bilinear");
    if (pszDEMInterpolation != nullptr)
        papszOptions = CSLSetNameValue(papszOptions, "RPC_DEMINTERPOLATION",
                                       pszDEMInterpolation);

    const char *pszDEMMissingValue =
        CPLGetXMLValue(psTree, "DEMMissingValue", nullptr);
    if (pszDEMMissingValue != nullptr)
        papszOptions = CSLSetNameValue(papszOptions, "RPC_DEM_MISSING_VALUE",
                                       pszDEMMissingValue);

    const char *pszDEMApplyVDatumShift =
        CPLGetXMLValue(psTree, "DEMApplyVDatumShift", nullptr);
    if (pszDEMApplyVDatumShift != nullptr)
        papszOptions = CSLSetNameValue(
            papszOptions, "RPC_DEM_APPLY_VDATUM_SHIFT", pszDEMApplyVDatumShift);
    const char *pszDEMSRS = CPLGetXMLValue(psTree, "DEMSRS", nullptr);
    if (pszDEMSRS != nullptr)
        papszOptions = CSLSetNameValue(papszOptions, "RPC_DEM_SRS", pszDEMSRS);

    /* -------------------------------------------------------------------- */
    /*      Generate transformation.                                        */
    /* -------------------------------------------------------------------- */
    void *pResult = GDALCreateRPCTransformerV2(&sRPC, bReversed,
                                               dfPixErrThreshold, papszOptions);

    CSLDestroy(papszOptions);

    return pResult;
}
