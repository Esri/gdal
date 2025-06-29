/******************************************************************************
 *
 * Project:  GeoTIFF Driver
 * Purpose:  Specialized copy of JPEG content into TIFF.
 * Author:   Even Rouault, <even dot rouault at spatialys.com>
 *
 ******************************************************************************
 * Copyright (c) 2012, Even Rouault <even dot rouault at spatialys.com>
 *
 * SPDX-License-Identifier: MIT
 ****************************************************************************/

#include "cpl_port.h"

#include "gt_jpeg_copy.h"

#include "cpl_vsi.h"

#if defined(JPEG_DIRECT_COPY) || defined(HAVE_LIBJPEG)
#include "vrt/vrtdataset.h"
#endif

#include <algorithm>

// Note: JPEG_DIRECT_COPY is not defined by default, because it is mainly
// useful for debugging purposes.

#if defined(JPEG_DIRECT_COPY) || defined(HAVE_LIBJPEG)

/************************************************************************/
/*                      GetUnderlyingDataset()                          */
/************************************************************************/

static GDALDataset *GetUnderlyingDataset(GDALDataset *poSrcDS)
{
    // Test if we can directly copy original JPEG content if available.
    if (auto poVRTDS = dynamic_cast<VRTDataset *>(poSrcDS))
    {
        poSrcDS = poVRTDS->GetSingleSimpleSource();
    }

    return poSrcDS;
}

#endif  // defined(JPEG_DIRECT_COPY) || defined(HAVE_LIBJPEG)

#ifdef JPEG_DIRECT_COPY

/************************************************************************/
/*                        IsBaselineDCTJPEG()                           */
/************************************************************************/

static bool IsBaselineDCTJPEG(VSILFILE *fp)
{
    GByte abyBuf[4] = {0};

    if (VSIFReadL(abyBuf, 1, 2, fp) != 2 || abyBuf[0] != 0xff ||
        abyBuf[1] != 0xd8)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Not a valid JPEG file");
        return false;
    }

    int nOffset = 2;
    while (true)
    {
        VSIFSeekL(fp, nOffset, SEEK_SET);
        if (VSIFReadL(abyBuf, 1, 4, fp) != 4 || abyBuf[0] != 0xFF)
        {
            CPLError(CE_Failure, CPLE_AppDefined, "Not a valid JPEG file");
            return false;
        }

        const int nMarker = abyBuf[1];

        // Start of Frame 0 = Baseline DCT.
        if (nMarker == 0xC0)
            return true;

        if (nMarker == 0xD9)
            return false;

        if (nMarker == 0xF7 ||  // JPEG Extension 7, JPEG-LS
            nMarker == 0xF8 ||  // JPEG Extension 8, JPEG-LS Extension.
            // Other Start of Frames that we don't want to support.
            (nMarker >= 0xC1 && nMarker <= 0xCF))
        {
            CPLError(CE_Failure, CPLE_AppDefined,
                     "Unsupported type of JPEG file for JPEG_DIRECT_COPY mode");
            return false;
        }

        nOffset += 2 + abyBuf[2] * 256 + abyBuf[3];
    }
}

/************************************************************************/
/*                    GTIFF_CanDirectCopyFromJPEG()                     */
/************************************************************************/

int GTIFF_CanDirectCopyFromJPEG(GDALDataset *poSrcDS,
                                char **&papszCreateOptions)
{
    poSrcDS = GetUnderlyingDataset(poSrcDS);
    if (poSrcDS == NULL)
        return FALSE;
    if (poSrcDS->GetDriver() == NULL)
        return FALSE;
    if (!EQUAL(GDALGetDriverShortName(poSrcDS->GetDriver()), "JPEG"))
        return FALSE;

    const char *pszCompress = CSLFetchNameValue(papszCreateOptions, "COMPRESS");
    if (pszCompress != NULL && !EQUAL(pszCompress, "JPEG"))
        return FALSE;

    const char *pszSrcColorSpace =
        poSrcDS->GetMetadataItem("SOURCE_COLOR_SPACE", "IMAGE_STRUCTURE");
    if (pszSrcColorSpace != NULL &&
        (EQUAL(pszSrcColorSpace, "CMYK") || EQUAL(pszSrcColorSpace, "YCbCrK")))
        return FALSE;

    bool bJPEGDirectCopy = false;

    VSILFILE *fpJPEG = VSIFOpenL(poSrcDS->GetDescription(), "rb");
    if (fpJPEG && IsBaselineDCTJPEG(fpJPEG))
    {
        bJPEGDirectCopy = true;

        if (pszCompress == NULL)
            papszCreateOptions =
                CSLSetNameValue(papszCreateOptions, "COMPRESS", "JPEG");

        papszCreateOptions =
            CSLSetNameValue(papszCreateOptions, "BLOCKXSIZE", NULL);
        papszCreateOptions =
            CSLSetNameValue(papszCreateOptions, "BLOCKYSIZE",
                            CPLSPrintf("%d", poSrcDS->GetRasterYSize()));

        if (pszSrcColorSpace != NULL && EQUAL(pszSrcColorSpace, "YCbCr"))
            papszCreateOptions =
                CSLSetNameValue(papszCreateOptions, "PHOTOMETRIC", "YCBCR");
        else
            papszCreateOptions =
                CSLSetNameValue(papszCreateOptions, "PHOTOMETRIC", NULL);

        if (poSrcDS->GetRasterBand(1)->GetRasterDataType() != GDT_Byte)
            papszCreateOptions =
                CSLSetNameValue(papszCreateOptions, "NBITS", "12");
        else
            papszCreateOptions =
                CSLSetNameValue(papszCreateOptions, "NBITS", NULL);

        papszCreateOptions = CSLSetNameValue(papszCreateOptions, "TILED", NULL);
        papszCreateOptions =
            CSLSetNameValue(papszCreateOptions, "JPEG_QUALITY", NULL);
    }
    if (fpJPEG)
    {
        CPL_IGNORE_RET_VAL(VSIFCloseL(fpJPEG));
    }

    return bJPEGDirectCopy;
}

/************************************************************************/
/*                     GTIFF_DirectCopyFromJPEG()                       */
/************************************************************************/

CPLErr GTIFF_DirectCopyFromJPEG(GDALDataset *poDS, GDALDataset *poSrcDS,
                                GDALProgressFunc pfnProgress,
                                void *pProgressData,
                                bool &bShouldFallbackToNormalCopyIfFail)
{
    bShouldFallbackToNormalCopyIfFail = true;

    poSrcDS = GetUnderlyingDataset(poSrcDS);
    if (poSrcDS == NULL)
        return CE_Failure;

    VSILFILE *fpJPEG = VSIFOpenL(poSrcDS->GetDescription(), "rb");
    if (fpJPEG == NULL)
        return CE_Failure;

    CPLErr eErr = CE_None;

    VSIFSeekL(fpJPEG, 0, SEEK_END);
    tmsize_t nSize = static_cast<tmsize_t>(VSIFTellL(fpJPEG));
    VSIFSeekL(fpJPEG, 0, SEEK_SET);

    void *pabyJPEGData = VSIMalloc(nSize);
    if (pabyJPEGData == NULL)
    {
        CPL_IGNORE_RET_VAL(VSIFCloseL(fpJPEG));
        return CE_Failure;
    }

    if (pabyJPEGData != NULL && static_cast<tmsize_t>(VSIFReadL(
                                    pabyJPEGData, 1, nSize, fpJPEG)) == nSize)
    {
        bShouldFallbackToNormalCopyIfFail = false;

        TIFF *hTIFF = (TIFF *)poDS->GetInternalHandle("TIFF_HANDLE");
        if (TIFFWriteRawStrip(hTIFF, 0, pabyJPEGData, nSize) != nSize)
            eErr = CE_Failure;

        if (!pfnProgress(1.0, NULL, pProgressData))
            eErr = CE_Failure;
    }
    else
    {
        eErr = CE_Failure;
    }

    VSIFree(pabyJPEGData);
    if (VSIFCloseL(fpJPEG) != 0)
        eErr = CE_Failure;

    return eErr;
}

#endif  // JPEG_DIRECT_COPY

#ifdef HAVE_LIBJPEG

#define jpeg_vsiio_src GTIFF_jpeg_vsiio_src
#define jpeg_vsiio_dest GTIFF_jpeg_vsiio_dest
#include "../jpeg/vsidataio.h"
#include "../jpeg/vsidataio.cpp"

#include <setjmp.h>

/*
 * We are using width_in_blocks which is supposed to be private to
 * libjpeg. Unfortunately, the libjpeg delivered with Cygwin has
 * renamed this member to width_in_data_units.  Since the header has
 * also renamed a define, use that unique define name in order to
 * detect the problem header and adjust to suit.
 */
#if defined(D_MAX_DATA_UNITS_IN_MCU)
#define width_in_blocks width_in_data_units
#endif

#ifdef EXPECTED_JPEG_LIB_VERSION
#if EXPECTED_JPEG_LIB_VERSION != JPEG_LIB_VERSION
#error EXPECTED_JPEG_LIB_VERSION != JPEG_LIB_VERSION
#endif
#endif

/************************************************************************/
/*                      GTIFF_CanCopyFromJPEG()                         */
/************************************************************************/

int GTIFF_CanCopyFromJPEG(GDALDataset *poSrcDS, char **&papszCreateOptions)
{
    poSrcDS = GetUnderlyingDataset(poSrcDS);
    if (poSrcDS == nullptr)
        return FALSE;
    if (poSrcDS->GetDriver() == nullptr)
        return FALSE;
    if (!EQUAL(GDALGetDriverShortName(poSrcDS->GetDriver()), "JPEG"))
        return FALSE;

    const char *pszCompress = CSLFetchNameValue(papszCreateOptions, "COMPRESS");
    if (pszCompress == nullptr || !EQUAL(pszCompress, "JPEG"))
        return FALSE;

    const int nBlockXSize =
        atoi(CSLFetchNameValueDef(papszCreateOptions, "BLOCKXSIZE", "0"));
    const int nBlockYSize =
        atoi(CSLFetchNameValueDef(papszCreateOptions, "BLOCKYSIZE", "0"));
    int nMCUSize = 8;
    const char *pszSrcColorSpace =
        poSrcDS->GetMetadataItem("SOURCE_COLOR_SPACE", "IMAGE_STRUCTURE");
    if (pszSrcColorSpace != nullptr && EQUAL(pszSrcColorSpace, "YCbCr"))
        nMCUSize = 16;

    const int nXSize = poSrcDS->GetRasterXSize();
    const int nYSize = poSrcDS->GetRasterYSize();
    const int nBands = poSrcDS->GetRasterCount();

    const char *pszPhotometric =
        CSLFetchNameValue(papszCreateOptions, "PHOTOMETRIC");

    const bool bCompatiblePhotometric =
        pszPhotometric == nullptr ||
        (nMCUSize == 16 && EQUAL(pszPhotometric, "YCbCr")) ||
        (nMCUSize == 8 && nBands == 4 &&
         poSrcDS->GetRasterBand(1)->GetColorInterpretation() == GCI_CyanBand &&
         poSrcDS->GetRasterBand(2)->GetColorInterpretation() ==
             GCI_MagentaBand &&
         poSrcDS->GetRasterBand(3)->GetColorInterpretation() ==
             GCI_YellowBand &&
         poSrcDS->GetRasterBand(4)->GetColorInterpretation() ==
             GCI_BlackBand) ||
        (nMCUSize == 8 && EQUAL(pszPhotometric, "RGB") && nBands == 3) ||
        (nMCUSize == 8 && EQUAL(pszPhotometric, "MINISBLACK") && nBands == 1);
    if (!bCompatiblePhotometric)
        return FALSE;

    if (nBands == 4 && pszPhotometric == nullptr &&
        poSrcDS->GetRasterBand(1)->GetColorInterpretation() == GCI_CyanBand &&
        poSrcDS->GetRasterBand(2)->GetColorInterpretation() ==
            GCI_MagentaBand &&
        poSrcDS->GetRasterBand(3)->GetColorInterpretation() == GCI_YellowBand &&
        poSrcDS->GetRasterBand(4)->GetColorInterpretation() == GCI_BlackBand)
    {
        papszCreateOptions =
            CSLSetNameValue(papszCreateOptions, "PHOTOMETRIC", "CMYK");
    }

    const char *pszInterleave =
        CSLFetchNameValue(papszCreateOptions, "INTERLEAVE");

    const bool bCompatibleInterleave =
        pszInterleave == nullptr ||
        (nBands > 1 && EQUAL(pszInterleave, "PIXEL")) || nBands == 1;
    if (!bCompatibleInterleave)
        return FALSE;

    // We don't want to apply lossy JPEG on a source using lossless JPEG !
    const char *pszReversibility = poSrcDS->GetMetadataItem(
        "COMPRESSION_REVERSIBILITY", "IMAGE_STRUCTURE");
    if (pszReversibility && EQUAL(pszReversibility, "LOSSLESS"))
        return FALSE;

    if ((nBlockXSize == nXSize || (nBlockXSize % nMCUSize) == 0) &&
        (nBlockYSize == nYSize || (nBlockYSize % nMCUSize) == 0) &&
        poSrcDS->GetRasterBand(1)->GetRasterDataType() == GDT_Byte &&
        CSLFetchNameValue(papszCreateOptions, "NBITS") == nullptr &&
        CSLFetchNameValue(papszCreateOptions, "JPEG_QUALITY") == nullptr)
    {
        if (nMCUSize == 16 && pszPhotometric == nullptr)
            papszCreateOptions =
                CSLSetNameValue(papszCreateOptions, "PHOTOMETRIC", "YCBCR");
        return TRUE;
    }

    return FALSE;
}

/************************************************************************/
/*                      GTIFF_ErrorExitJPEG()                           */
/************************************************************************/

static void GTIFF_ErrorExitJPEG(j_common_ptr cinfo)
{
    jmp_buf *setjmp_buffer = static_cast<jmp_buf *>(cinfo->client_data);
    char buffer[JMSG_LENGTH_MAX] = {'\0'};

    // Create the message.
    (*cinfo->err->format_message)(cinfo, buffer);

    CPLError(CE_Failure, CPLE_AppDefined, "libjpeg: %s", buffer);

    // Return control to the setjmp point.
    longjmp(*setjmp_buffer, 1);
}

/************************************************************************/
/*                      GTIFF_Set_TIFFTAG_JPEGTABLES()                  */
/************************************************************************/

static void GTIFF_Set_TIFFTAG_JPEGTABLES(TIFF *hTIFF,
                                         jpeg_compress_struct &sCInfo)
{
    const std::string osTmpFilename(VSIMemGenerateHiddenFilename("tables"));
    VSILFILE *fpTABLES = VSIFOpenL(osTmpFilename.c_str(), "wb+");

    uint16_t nPhotometric = 0;
    TIFFGetField(hTIFF, TIFFTAG_PHOTOMETRIC, &nPhotometric);

    jpeg_vsiio_dest(&sCInfo, fpTABLES);

    // Avoid unnecessary tables to be emitted.
    if (nPhotometric != PHOTOMETRIC_YCBCR)
    {
        JQUANT_TBL *qtbl = sCInfo.quant_tbl_ptrs[1];
        if (qtbl != nullptr)
            qtbl->sent_table = TRUE;
        JHUFF_TBL *htbl = sCInfo.dc_huff_tbl_ptrs[1];
        if (htbl != nullptr)
            htbl->sent_table = TRUE;
        htbl = sCInfo.ac_huff_tbl_ptrs[1];
        if (htbl != nullptr)
            htbl->sent_table = TRUE;
    }
    jpeg_write_tables(&sCInfo);

    CPL_IGNORE_RET_VAL(VSIFCloseL(fpTABLES));

    vsi_l_offset nSizeTables = 0;
    GByte *pabyJPEGTablesData =
        VSIGetMemFileBuffer(osTmpFilename.c_str(), &nSizeTables, FALSE);
    TIFFSetField(hTIFF, TIFFTAG_JPEGTABLES, static_cast<int>(nSizeTables),
                 pabyJPEGTablesData);

    VSIUnlink(osTmpFilename.c_str());
}

/************************************************************************/
/*             GTIFF_CopyFromJPEG_WriteAdditionalTags()                 */
/************************************************************************/

CPLErr GTIFF_CopyFromJPEG_WriteAdditionalTags(TIFF *hTIFF, GDALDataset *poSrcDS)
{
    poSrcDS = GetUnderlyingDataset(poSrcDS);
    if (poSrcDS == nullptr)
        return CE_Failure;

    /* -------------------------------------------------------------------- */
    /*      Write TIFFTAG_JPEGTABLES                                        */
    /* -------------------------------------------------------------------- */

    VSILFILE *fpJPEG = VSIFOpenL(poSrcDS->GetDescription(), "rb");
    if (fpJPEG == nullptr)
        return CE_Failure;

    struct jpeg_error_mgr sJErr;
    struct jpeg_decompress_struct sDInfo;
    jmp_buf setjmp_buffer;

    volatile bool bCallDestroyDecompress = false;
    volatile bool bCallDestroyCompress = false;

    struct jpeg_compress_struct sCInfo;

    if (setjmp(setjmp_buffer))
    {
        if (bCallDestroyCompress)
        {
            jpeg_abort_compress(&sCInfo);
            jpeg_destroy_compress(&sCInfo);
        }
        if (bCallDestroyDecompress)
        {
            jpeg_abort_decompress(&sDInfo);
            jpeg_destroy_decompress(&sDInfo);
        }
        CPL_IGNORE_RET_VAL(VSIFCloseL(fpJPEG));
        return CE_Failure;
    }

    sDInfo.err = jpeg_std_error(&sJErr);
    sJErr.error_exit = GTIFF_ErrorExitJPEG;
    sDInfo.client_data = &setjmp_buffer;

    bCallDestroyDecompress = true;
    jpeg_CreateDecompress(&sDInfo, JPEG_LIB_VERSION, sizeof(sDInfo));

    jpeg_vsiio_src(&sDInfo, fpJPEG);
    jpeg_read_header(&sDInfo, TRUE);

    sCInfo.err = jpeg_std_error(&sJErr);
    sJErr.error_exit = GTIFF_ErrorExitJPEG;
    sCInfo.client_data = &setjmp_buffer;

    jpeg_CreateCompress(&sCInfo, JPEG_LIB_VERSION, sizeof(sCInfo));
    bCallDestroyCompress = true;
    jpeg_copy_critical_parameters(&sDInfo, &sCInfo);
    GTIFF_Set_TIFFTAG_JPEGTABLES(hTIFF, sCInfo);
    bCallDestroyCompress = false;
    jpeg_abort_compress(&sCInfo);
    jpeg_destroy_compress(&sCInfo);
    CPL_IGNORE_RET_VAL(bCallDestroyCompress);

    /* -------------------------------------------------------------------- */
    /*      Write TIFFTAG_REFERENCEBLACKWHITE if needed.                    */
    /* -------------------------------------------------------------------- */

    uint16_t nPhotometric = 0;
    if (!TIFFGetField(hTIFF, TIFFTAG_PHOTOMETRIC, &(nPhotometric)))
        nPhotometric = PHOTOMETRIC_MINISBLACK;

    uint16_t nBitsPerSample = 0;
    if (!TIFFGetField(hTIFF, TIFFTAG_BITSPERSAMPLE, &(nBitsPerSample)))
        nBitsPerSample = 1;

    if (nPhotometric == PHOTOMETRIC_YCBCR)
    {
        /*
         * A ReferenceBlackWhite field *must* be present since the
         * default value is inappropriate for YCbCr.  Fill in the
         * proper value if application didn't set it.
         */
        float *ref = nullptr;
        if (!TIFFGetField(hTIFF, TIFFTAG_REFERENCEBLACKWHITE, &ref))
        {
            long top = 1L << nBitsPerSample;
            float refbw[6] = {0.0};
            refbw[1] = static_cast<float>(top - 1L);
            refbw[2] = static_cast<float>(top >> 1);
            refbw[3] = refbw[1];
            refbw[4] = refbw[2];
            refbw[5] = refbw[1];
            TIFFSetField(hTIFF, TIFFTAG_REFERENCEBLACKWHITE, refbw);
        }
    }

    /* -------------------------------------------------------------------- */
    /*      Write TIFFTAG_YCBCRSUBSAMPLING if needed.                       */
    /* -------------------------------------------------------------------- */

    if (nPhotometric == PHOTOMETRIC_YCBCR && sDInfo.num_components == 3)
    {
        if ((sDInfo.comp_info[0].h_samp_factor == 1 ||
             sDInfo.comp_info[0].h_samp_factor == 2) &&
            (sDInfo.comp_info[0].v_samp_factor == 1 ||
             sDInfo.comp_info[0].v_samp_factor == 2) &&
            sDInfo.comp_info[1].h_samp_factor == 1 &&
            sDInfo.comp_info[1].v_samp_factor == 1 &&
            sDInfo.comp_info[2].h_samp_factor == 1 &&
            sDInfo.comp_info[2].v_samp_factor == 1)
        {
            TIFFSetField(hTIFF, TIFFTAG_YCBCRSUBSAMPLING,
                         sDInfo.comp_info[0].h_samp_factor,
                         sDInfo.comp_info[0].v_samp_factor);
        }
        else
        {
            CPLDebug("GTiff", "Unusual sampling factors. "
                              "TIFFTAG_YCBCRSUBSAMPLING not written.");
        }
    }

    /* -------------------------------------------------------------------- */
    /*      Cleanup.                                                        */
    /* -------------------------------------------------------------------- */

    bCallDestroyDecompress = false;
    jpeg_abort_decompress(&sDInfo);
    jpeg_destroy_decompress(&sDInfo);
    CPL_IGNORE_RET_VAL(bCallDestroyDecompress);

    if (VSIFCloseL(fpJPEG) != 0)
        return CE_Failure;

    return CE_None;
}

/************************************************************************/
/*                    GTIFF_CopyBlockFromJPEG()                         */
/************************************************************************/

typedef struct
{
    TIFF *hTIFF;
    jpeg_decompress_struct *psDInfo;
    int iX;
    int iY;
    int nXBlocks;
    int nXSize;
    int nYSize;
    int nBlockXSize;
    int nBlockYSize;
    int iMCU_sample_width;
    int iMCU_sample_height;
    jvirt_barray_ptr *pSrcCoeffs;
} GTIFF_CopyBlockFromJPEGArgs;

static CPLErr GTIFF_CopyBlockFromJPEG(GTIFF_CopyBlockFromJPEGArgs *psArgs)
{
    const CPLString osTmpFilename(
        VSIMemGenerateHiddenFilename("GTIFF_CopyBlockFromJPEG.tif"));
    VSILFILE *fpMEM = VSIFOpenL(osTmpFilename.c_str(), "wb+");

    /* -------------------------------------------------------------------- */
    /*      Initialization of the compressor                                */
    /* -------------------------------------------------------------------- */
    jmp_buf setjmp_buffer;
    if (setjmp(setjmp_buffer))
    {
        CPL_IGNORE_RET_VAL(VSIFCloseL(fpMEM));
        VSIUnlink(osTmpFilename.c_str());
        return CE_Failure;
    }

    TIFF *hTIFF = psArgs->hTIFF;
    jpeg_decompress_struct *psDInfo = psArgs->psDInfo;
    const int iX = psArgs->iX;
    const int iY = psArgs->iY;
    const int nXBlocks = psArgs->nXBlocks;
    const int nXSize = psArgs->nXSize;
    const int nYSize = psArgs->nYSize;
    const int nBlockXSize = psArgs->nBlockXSize;
    const int nBlockYSize = psArgs->nBlockYSize;
    const int iMCU_sample_width = psArgs->iMCU_sample_width;
    const int iMCU_sample_height = psArgs->iMCU_sample_height;
    jvirt_barray_ptr *pSrcCoeffs = psArgs->pSrcCoeffs;

    struct jpeg_error_mgr sJErr;
    struct jpeg_compress_struct sCInfo;
    sCInfo.err = jpeg_std_error(&sJErr);
    sJErr.error_exit = GTIFF_ErrorExitJPEG;
    sCInfo.client_data = &setjmp_buffer;

    // Initialize destination compression parameters from source values.
    jpeg_CreateCompress(&sCInfo, JPEG_LIB_VERSION, sizeof(sCInfo));
    jpeg_copy_critical_parameters(psDInfo, &sCInfo);

    // Ensure libjpeg won't write any extraneous markers.
    sCInfo.write_JFIF_header = FALSE;
    sCInfo.write_Adobe_marker = FALSE;

    /* -------------------------------------------------------------------- */
    /*      Allocated destination coefficient array                         */
    /* -------------------------------------------------------------------- */
    const bool bIsTiled = CPL_TO_BOOL(TIFFIsTiled(hTIFF));

    int nJPEGWidth = nBlockXSize;
    int nJPEGHeight = nBlockYSize;
    if (!bIsTiled)
    {
        nJPEGWidth = std::min(nBlockXSize, nXSize - iX * nBlockXSize);
        nJPEGHeight = std::min(nBlockYSize, nYSize - iY * nBlockYSize);
    }

// Code partially derived from libjpeg transupp.c.

// Correct the destination's image dimensions as necessary.
#if JPEG_LIB_VERSION >= 70
    sCInfo.jpeg_width = nJPEGWidth;
    sCInfo.jpeg_height = nJPEGHeight;
#else
    sCInfo.image_width = nJPEGWidth;
    sCInfo.image_height = nJPEGHeight;
#endif

    // Save x/y offsets measured in iMCUs.
    const int x_crop_offset = (iX * nBlockXSize) / iMCU_sample_width;
    const int y_crop_offset = (iY * nBlockYSize) / iMCU_sample_height;

    jvirt_barray_ptr *pDstCoeffs =
        static_cast<jvirt_barray_ptr *>((*sCInfo.mem->alloc_small)(
            reinterpret_cast<j_common_ptr>(&sCInfo), JPOOL_IMAGE,
            sizeof(jvirt_barray_ptr) * sCInfo.num_components));

    for (int ci = 0; ci < sCInfo.num_components; ci++)
    {
        jpeg_component_info *compptr = sCInfo.comp_info + ci;
        int h_samp_factor, v_samp_factor;
        if (sCInfo.num_components == 1)
        {
            // Force samp factors to 1x1 in this case.
            h_samp_factor = 1;
            v_samp_factor = 1;
        }
        else
        {
            h_samp_factor = compptr->h_samp_factor;
            v_samp_factor = compptr->v_samp_factor;
        }
        int width_in_iMCUs = DIV_ROUND_UP(nJPEGWidth, iMCU_sample_width);
        int height_in_iMCUs = DIV_ROUND_UP(nJPEGHeight, iMCU_sample_height);
        int nWidth_in_blocks = width_in_iMCUs * h_samp_factor;
        int nHeight_in_blocks = height_in_iMCUs * v_samp_factor;
        pDstCoeffs[ci] = (*sCInfo.mem->request_virt_barray)(
            reinterpret_cast<j_common_ptr>(&sCInfo), JPOOL_IMAGE, FALSE,
            nWidth_in_blocks, nHeight_in_blocks,
            static_cast<JDIMENSION>(v_samp_factor));
    }

    jpeg_vsiio_dest(&sCInfo, fpMEM);

    // Start compressor (note no image data is actually written here).
    jpeg_write_coefficients(&sCInfo, pDstCoeffs);

    jpeg_suppress_tables(&sCInfo, TRUE);

    // Must copy the right amount of data (the destination's image size)
    // starting at the given X and Y offsets in the source.
    for (int ci = 0; ci < sCInfo.num_components; ci++)
    {
        jpeg_component_info *compptr = sCInfo.comp_info + ci;
        const int x_crop_blocks = x_crop_offset * compptr->h_samp_factor;
        const int y_crop_blocks = y_crop_offset * compptr->v_samp_factor;
        const JDIMENSION nSrcWidthInBlocks =
            psDInfo->comp_info[ci].width_in_blocks;
        const JDIMENSION nSrcHeightInBlocks =
            psDInfo->comp_info[ci].height_in_blocks;

        JDIMENSION nXBlocksToCopy = compptr->width_in_blocks;
        if (x_crop_blocks + compptr->width_in_blocks > nSrcWidthInBlocks)
            nXBlocksToCopy = nSrcWidthInBlocks - x_crop_blocks;

        for (JDIMENSION dst_blk_y = 0; dst_blk_y < compptr->height_in_blocks;
             dst_blk_y += compptr->v_samp_factor)
        {
            JBLOCKARRAY dst_buffer = (*psDInfo->mem->access_virt_barray)(
                reinterpret_cast<j_common_ptr>(psDInfo), pDstCoeffs[ci],
                dst_blk_y, static_cast<JDIMENSION>(compptr->v_samp_factor),
                TRUE);

            int offset_y = 0;
            if (bIsTiled && dst_blk_y + y_crop_blocks + compptr->v_samp_factor >
                                nSrcHeightInBlocks)
            {
                const int nYBlocks =
                    static_cast<int>(nSrcHeightInBlocks) -
                    static_cast<int>(dst_blk_y + y_crop_blocks);
                if (nYBlocks > 0)
                {
                    JBLOCKARRAY src_buffer =
                        (*psDInfo->mem->access_virt_barray)(
                            reinterpret_cast<j_common_ptr>(psDInfo),
                            pSrcCoeffs[ci], dst_blk_y + y_crop_blocks,
                            static_cast<JDIMENSION>(1), FALSE);
                    for (; offset_y < nYBlocks; offset_y++)
                    {
                        memcpy(dst_buffer[offset_y],
                               src_buffer[offset_y] + x_crop_blocks,
                               nXBlocksToCopy * (DCTSIZE2 * sizeof(JCOEF)));
                        if (nXBlocksToCopy < compptr->width_in_blocks)
                        {
                            memset(dst_buffer[offset_y] + nXBlocksToCopy, 0,
                                   (compptr->width_in_blocks - nXBlocksToCopy) *
                                       (DCTSIZE2 * sizeof(JCOEF)));
                        }
                    }
                }

                for (; offset_y < compptr->v_samp_factor; offset_y++)
                {
                    memset(dst_buffer[offset_y], 0,
                           compptr->width_in_blocks * DCTSIZE2 * sizeof(JCOEF));
                }
            }
            else
            {
                JBLOCKARRAY src_buffer = (*psDInfo->mem->access_virt_barray)(
                    reinterpret_cast<j_common_ptr>(psDInfo), pSrcCoeffs[ci],
                    dst_blk_y + y_crop_blocks,
                    static_cast<JDIMENSION>(compptr->v_samp_factor), FALSE);
                for (; offset_y < compptr->v_samp_factor; offset_y++)
                {
                    memcpy(dst_buffer[offset_y],
                           src_buffer[offset_y] + x_crop_blocks,
                           nXBlocksToCopy * (DCTSIZE2 * sizeof(JCOEF)));
                    if (nXBlocksToCopy < compptr->width_in_blocks)
                    {
                        memset(dst_buffer[offset_y] + nXBlocksToCopy, 0,
                               (compptr->width_in_blocks - nXBlocksToCopy) *
                                   (DCTSIZE2 * sizeof(JCOEF)));
                    }
                }
            }
        }
    }

    jpeg_finish_compress(&sCInfo);
    jpeg_destroy_compress(&sCInfo);

    CPL_IGNORE_RET_VAL(VSIFCloseL(fpMEM));

    /* -------------------------------------------------------------------- */
    /*      Write the JPEG content with libtiff raw API                     */
    /* -------------------------------------------------------------------- */
    vsi_l_offset nSize = 0;
    GByte *pabyJPEGData =
        VSIGetMemFileBuffer(osTmpFilename.c_str(), &nSize, FALSE);

    CPLErr eErr = CE_None;

    if (bIsTiled)
    {
        if (static_cast<vsi_l_offset>(
                TIFFWriteRawTile(hTIFF, iX + iY * nXBlocks, pabyJPEGData,
                                 static_cast<tmsize_t>(nSize))) != nSize)
            eErr = CE_Failure;
    }
    else
    {
        if (static_cast<vsi_l_offset>(
                TIFFWriteRawStrip(hTIFF, iX + iY * nXBlocks, pabyJPEGData,
                                  static_cast<tmsize_t>(nSize))) != nSize)
            eErr = CE_Failure;
    }

    VSIUnlink(osTmpFilename.c_str());

    return eErr;
}

/************************************************************************/
/*                      GTIFF_CopyFromJPEG()                            */
/************************************************************************/

CPLErr GTIFF_CopyFromJPEG(GDALDataset *poDS, GDALDataset *poSrcDS,
                          GDALProgressFunc pfnProgress, void *pProgressData,
                          bool &bShouldFallbackToNormalCopyIfFail)
{
    bShouldFallbackToNormalCopyIfFail = true;

    poSrcDS = GetUnderlyingDataset(poSrcDS);
    if (poSrcDS == nullptr)
        return CE_Failure;

    VSILFILE *fpJPEG = VSIFOpenL(poSrcDS->GetDescription(), "rb");
    if (fpJPEG == nullptr)
        return CE_Failure;

    CPLErr eErr = CE_None;

    /* -------------------------------------------------------------------- */
    /*      Initialization of the decompressor                              */
    /* -------------------------------------------------------------------- */
    struct jpeg_error_mgr sJErr;
    struct jpeg_decompress_struct sDInfo;
    memset(&sDInfo, 0, sizeof(sDInfo));
    jmp_buf setjmp_buffer;
    if (setjmp(setjmp_buffer))
    {
        CPL_IGNORE_RET_VAL(VSIFCloseL(fpJPEG));
        jpeg_destroy_decompress(&sDInfo);
        return CE_Failure;
    }

    sDInfo.err = jpeg_std_error(&sJErr);
    sJErr.error_exit = GTIFF_ErrorExitJPEG;
    sDInfo.client_data = &setjmp_buffer;

    jpeg_CreateDecompress(&sDInfo, JPEG_LIB_VERSION, sizeof(sDInfo));

    // This is to address bug related in ticket #1795.
    if (CPLGetConfigOption("JPEGMEM", nullptr) == nullptr)
    {
        // If the user doesn't provide a value for JPEGMEM, be sure that at
        // least 500 MB will be used before creating the temporary file.
        const long nMinMemory = 500 * 1024 * 1024;
        sDInfo.mem->max_memory_to_use =
            std::max(sDInfo.mem->max_memory_to_use, nMinMemory);
    }

    jpeg_vsiio_src(&sDInfo, fpJPEG);
    jpeg_read_header(&sDInfo, TRUE);

    jvirt_barray_ptr *pSrcCoeffs = jpeg_read_coefficients(&sDInfo);

    /* -------------------------------------------------------------------- */
    /*      Compute MCU dimensions                                          */
    /* -------------------------------------------------------------------- */
    int iMCU_sample_width = 8;
    int iMCU_sample_height = 8;
    if (sDInfo.num_components != 1)
    {
        iMCU_sample_width = sDInfo.max_h_samp_factor * 8;
        iMCU_sample_height = sDInfo.max_v_samp_factor * 8;
    }

    /* -------------------------------------------------------------------- */
    /*      Get raster and block dimensions                                 */
    /* -------------------------------------------------------------------- */
    int nBlockXSize = 0;
    int nBlockYSize = 0;

    const int nXSize = poDS->GetRasterXSize();
    const int nYSize = poDS->GetRasterYSize();
    // nBands = poDS->GetRasterCount();

    // Don't use the GDAL block dimensions because of the split-band
    // mechanism that can expose a pseudo one-line-strip whereas the
    // real layout is a single big strip.

    TIFF *hTIFF = static_cast<TIFF *>(poDS->GetInternalHandle("TIFF_HANDLE"));
    if (TIFFIsTiled(hTIFF))
    {
        TIFFGetField(hTIFF, TIFFTAG_TILEWIDTH, &(nBlockXSize));
        TIFFGetField(hTIFF, TIFFTAG_TILELENGTH, &(nBlockYSize));
    }
    else
    {
        uint32_t nRowsPerStrip = 0;
        if (!TIFFGetField(hTIFF, TIFFTAG_ROWSPERSTRIP, &(nRowsPerStrip)))
        {
            CPLError(CE_Warning, CPLE_AppDefined,
                     "RowsPerStrip not defined ... assuming all one strip.");
            nRowsPerStrip = nYSize;  // Dummy value.
        }

        // If the rows per strip is larger than the file we will get
        // confused.  libtiff internally will treat the rowsperstrip as
        // the image height and it is best if we do too. (#4468)
        if (nRowsPerStrip > static_cast<uint32_t>(nYSize))
            nRowsPerStrip = nYSize;

        nBlockXSize = nXSize;
        nBlockYSize = nRowsPerStrip;
    }

    const int nXBlocks = DIV_ROUND_UP(nXSize, nBlockXSize);
    const int nYBlocks = DIV_ROUND_UP(nYSize, nBlockYSize);

    /* -------------------------------------------------------------------- */
    /*      Copy blocks.                                                    */
    /* -------------------------------------------------------------------- */

    bShouldFallbackToNormalCopyIfFail = false;

    for (int iY = 0; iY < nYBlocks && eErr == CE_None; iY++)
    {
        for (int iX = 0; iX < nXBlocks && eErr == CE_None; iX++)
        {
            GTIFF_CopyBlockFromJPEGArgs sArgs;
            sArgs.hTIFF = hTIFF;
            sArgs.psDInfo = &sDInfo;
            sArgs.iX = iX;
            sArgs.iY = iY;
            sArgs.nXBlocks = nXBlocks;
            sArgs.nXSize = nXSize;
            sArgs.nYSize = nYSize;
            sArgs.nBlockXSize = nBlockXSize;
            sArgs.nBlockYSize = nBlockYSize;
            sArgs.iMCU_sample_width = iMCU_sample_width;
            sArgs.iMCU_sample_height = iMCU_sample_height;
            sArgs.pSrcCoeffs = pSrcCoeffs;

            eErr = GTIFF_CopyBlockFromJPEG(&sArgs);

            if (!pfnProgress((iY * nXBlocks + iX + 1) * 1.0 /
                                 (nXBlocks * nYBlocks),
                             nullptr, pProgressData))
                eErr = CE_Failure;
        }
    }

    /* -------------------------------------------------------------------- */
    /*      Cleanup.                                                        */
    /* -------------------------------------------------------------------- */

    jpeg_finish_decompress(&sDInfo);
    jpeg_destroy_decompress(&sDInfo);

    if (VSIFCloseL(fpJPEG) != 0)
        eErr = CE_Failure;

    return eErr;
}

#endif  // HAVE_LIBJPEG
