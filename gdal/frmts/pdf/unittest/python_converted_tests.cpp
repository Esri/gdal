#include "CppUnitTest.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include <chrono>
#include <filesystem>

#include "gdal/gdal.h"
#include "gdal/gdal_priv.h"
#include "ogr/ogrsf_frmts.h"
#include "test_utilities.h"
#include "cpl_conv.h"
#include "ogr_spatialref.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace gdalpdfunittest
{
    TEST_CLASS(Tests_Python_Converted_PDF_Online)
    {
    public:
        TEST_METHOD(Test_PDF_Online_1)
        {
            const std::string remote_url =
                "http://www.agc.army.mil/GeoPDFgallery/Imagery/Cherrydale_eDOQQ_1m_0_033_R1C1.pdf";
            const std::string local_name = "Cherrydale_eDOQQ_1m_0_033_R1C1.pdf";
            const std::string local_path = "tmp/cache/" + local_name;

            // Check driver
            GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("PDF");
            Assert::IsNotNull(driver, L"PDF driver is not available");

            // Download file if needed
            if (!download_file(remote_url.c_str(), local_name.c_str()))
                Assert::Fail(L"File download failed");

            // Ensure file exists
            if (!file_exists(local_path.c_str()))
                Assert::Fail(L"Downloaded file does not exist");

            // Open dataset
            GDALDataset* ds = static_cast<GDALDataset*>(GDALOpen(local_path.c_str(), GA_ReadOnly));
            Assert::IsNotNull(ds, L"Failed to open GeoPDF");

            // Validate raster size
            Assert::AreEqual(1241, ds->GetRasterXSize(), L"Unexpected raster width");

            // Validate GeoTransform
            double gt[6] = {};
            if (ds->GetGeoTransform(gt) != CE_None)
                Assert::Fail(L"Failed to get GeoTransform");

            double expected_gt[6];
            if (pdf_is_pdfium())
            {
                double pdfium_gt[6] = {
                    -77.11232757568358, 9.1663393281356228e-06, 0.0,
                    38.897842406247477, 0.0, -9.1665025563464202e-06
                };
                std::copy(std::begin(pdfium_gt), std::end(pdfium_gt), expected_gt);
            }
            else if (pdf_is_poppler())
            {
                double poppler_gt[6] = {
                    -77.112328333299999, 9.1666559999999995e-06, 0.0,
                    38.897842488372, -0.0, -9.1666559999999995e-06
                };
                std::copy(std::begin(poppler_gt), std::end(poppler_gt), expected_gt);
            }
            else
            {
                double fallback_gt[6] = {
                    -77.112328333299956, 9.1666560000051172e-06, 0.0,
                    38.897842488371978, 0.0, -9.1666560000046903e-06
                };
                std::copy(std::begin(fallback_gt), std::end(fallback_gt), expected_gt);
            }

            bool match = true;
            for (int i = 0; i < 6; ++i)
            {
                if (std::abs(gt[i] - expected_gt[i]) > 1e-15)
                {
                    // Try fallback (remote file might have been updated)
                    double other_expected_gt[6] = {
                        -77.112328333299928, 9.1666560000165691e-06, 0.0,
                        38.897842488371978, 0.0, -9.1666560000046903e-06
                    };
                    if (std::abs(gt[i] - other_expected_gt[i]) > 1e-15)
                    {
                        match = false;
                        break;
                    }
                }
            }
            Assert::IsTrue(match, L"GeoTransform mismatch");

            const char* wkt = ds->GetProjectionRef();
            Assert::IsTrue(wkt != nullptr && std::string(wkt).find("GEOGCS[\"WGS 84\"") == 0, L"Unexpected WKT");

            // Check checksum
            //if (pdf_checksum_available())
            //{
            //    int checksum = ds->GetRasterBand(1)->Checksum();
            //    Assert::IsTrue(checksum != 0, L"Invalid checksum: image may not be rendered");
            //}

            GDALClose(ds);
        }

        TEST_METHOD(Test_PDF_Online_2)
        {
            const std::string filename = "tmp/cache/Cherrydale_eDOQQ_1m_0_033_R1C1.pdf";
            if (!file_exists(filename.c_str()))
            {
                Logger::WriteMessage("Skipping test: file does not exist");
                return;
            }

            std::string fullPath = "PDF:1:" + filename;
            GDALDataset* poDS = static_cast<GDALDataset*>(GDALOpen(fullPath.c_str(), GA_ReadOnly));
            Assert::IsNotNull(poDS, L"Failed to open dataset");

            double gt[6];
            if (poDS->GetGeoTransform(gt) != CE_None)
            {
                GDALClose(poDS);
                Assert::Fail(L"Failed to get geotransform");
            }

            const char* wkt = poDS->GetProjectionRef();
            Assert::IsNotNull(wkt, L"Projection string is null");

            double expected_gt[6] = {};
            if (pdf_is_pdfium())
            {
                double tmp[] = { -77.11232757568358, 9.1663393281356228e-06, 0.0, 38.897842406247477, 0.0, -9.1665025563464202e-06 };
                std::copy(tmp, tmp + 6, expected_gt);
            }
            else if (pdf_is_poppler())
            {
                double tmp[] = { -77.112328333299999, 9.1666559999999995e-06, 0.0, 38.897842488372, -0.0, -9.1666559999999995e-06 };
                std::copy(tmp, tmp + 6, expected_gt);
            }
            else
            {
                double tmp[] = { -77.112328333299956, 9.1666560000051172e-06, 0.0, 38.897842488371978, 0.0, -9.1666560000046903e-06 };
                std::copy(tmp, tmp + 6, expected_gt);
            }

            bool match = true;
            for (int i = 0; i < 6; ++i)
            {
                if (std::abs(gt[i] - expected_gt[i]) > 1e-15)
                {
                    double fallback_gt[] = { -77.112328333299928, 9.1666560000165691e-06, 0.0, 38.897842488371978, 0.0, -9.1666560000046903e-06 };
                    if (std::abs(gt[i] - fallback_gt[i]) > 1e-15)
                    {
                        match = false;
                        break;
                    }
                }
            }

            Assert::IsTrue(match, L"GeoTransform mismatch");

            std::string wktStr(wkt);
            Assert::IsTrue(wktStr.find("GEOGCS[\"WGS 84\"") == 0, L"Unexpected WKT");

            GDALClose(poDS);
        }

        TEST_METHOD(Test_pdf_1)
        {
            // Set DPI
            CPLSetConfigOption("GDAL_PDF_DPI", "200");
            GDALDataset* poDS = static_cast<GDALDataset*>(GDALOpen("../../testdata/adobe_style_geospatial.pdf", GA_ReadOnly));
            CPLSetConfigOption("GDAL_PDF_DPI", nullptr);
            Assert::IsNotNull(poDS, L"Dataset could not be opened");

            // Geotransform check
            double gt[6];
            CPLErr err = poDS->GetGeoTransform(gt);
            Assert::AreEqual(CE_None, err, L"Failed to get geotransform");

            // WKT check
            const char* wkt = poDS->GetProjectionRef();
            Assert::IsNotNull(wkt, L"WKT string is null");

            double expected_gt[6];
            if (pdf_is_pdfium())
            {
                double tmp[] = { 333275.12406585668, 31.764450118407499, 0.0, 4940392.1233656602, 0.0, -31.794983670894396 };
                std::copy(tmp, tmp + 6, expected_gt);
            }
            else
            {
                double tmp[] = { 333274.61654367246, 31.764802242655662, 0.0, 4940391.7593506984, 0.0, -31.794745501708238 };
                std::copy(tmp, tmp + 6, expected_gt);
            }

            for (int i = 0; i < 6; ++i)
            {
                Assert::IsTrue(std::abs(gt[i] - expected_gt[i]) <= 1e-6, L"GeoTransform mismatch");
            }

            std::string expected_wkt =
                "PROJCS[\"WGS_1984_UTM_Zone_20N\",GEOGCS[\"GCS_WGS_1984\",DATUM[\"WGS_1984\","
                "SPHEROID[\"WGS_84\",6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],"
                "UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],"
                "PARAMETER[\"False_Easting\",500000.0],PARAMETER[\"False_Northing\",0.0],"
                "PARAMETER[\"Central_Meridian\",-63.0],PARAMETER[\"Scale_Factor\",0.9996],"
                "PARAMETER[\"Latitude_Of_Origin\",0.0],UNIT[\"Meter\",1.0]]";

            Assert::AreEqual(expected_wkt, std::string(wkt), L"WKT mismatch");

            /*if (pdf_checksum_available())
            {
                int checksum = poDS->GetRasterBand(1)->Checksum();
                Assert::IsTrue(checksum != 0, L"Invalid checksum");
            }*/

            const char* neatline = poDS->GetMetadataItem("NEATLINE");
            Assert::IsNotNull(neatline, L"Missing NEATLINE metadata");

            OGRGeometry* got_geom = nullptr;
            OGRErr ogrErr = OGRGeometryFactory::createFromWkt(neatline, nullptr, &got_geom);
            Assert::AreEqual(OGRERR_NONE, ogrErr, L"Could not parse NEATLINE geometry");
            Assert::IsNotNull(got_geom, L"Got null geometry");

            OGRGeometry* expected_geom = nullptr;
            const char* wkt_pdfium = "POLYGON ((338304.28536533244187 4896674.10591614805162,338304.812550922040828 4933414.853961281478405,382774.246895745047368 4933414.855149634182453,382774.983309225703124 4896673.95723026804626,338304.28536533244187 4896674.10591614805162))";
            const char* wkt_poppler = "POLYGON ((338304.150125828920864 4896673.639421294443309,338304.177293475600891 4933414.799376524984837,382774.271384406310972 4933414.546264361590147,382774.767329963855445 4896674.273581005632877,338304.150125828920864 4896673.639421294443309))";

            const char* expected_wkt_geom = pdf_is_pdfium() ? wkt_pdfium : wkt_poppler;
            ogrErr = OGRGeometryFactory::createFromWkt(expected_wkt_geom, nullptr, &expected_geom);
            Assert::AreEqual(OGRERR_NONE, ogrErr, L"Could not create expected NEATLINE geometry");
            Assert::IsNotNull(expected_geom, L"Expected geometry is null");

            Assert::IsTrue(got_geom->Equals(expected_geom), L"NEATLINE geometry mismatch");

            OGRGeometryFactory::destroyGeometry(got_geom);
            OGRGeometryFactory::destroyGeometry(expected_geom);
            GDALClose(poDS);
        }

        TEST_METHOD(Test_pdf_iso32000)
        {
            GDALAllRegister();

            const char* inputPath = "../../testdata/byte.tif";
            const char* outputPath = "../../testdata/output_iso32000.pdf";

            // Open source dataset
            GDALDataset* poSrcDS = static_cast<GDALDataset*>(GDALOpen(inputPath, GA_ReadOnly));
            Assert::IsNotNull(poSrcDS, L"Failed to open source dataset");

            GDALDriver* poPDFDriver = GetGDALDriverManager()->GetDriverByName("PDF");
            Assert::IsNotNull(poPDFDriver, L"PDF driver not available");

            char** papszOptions = nullptr;
            GDALDataset* poOutDS = poPDFDriver->CreateCopy(outputPath, poSrcDS, FALSE, papszOptions, nullptr, nullptr);
            Assert::IsNotNull(poOutDS, L"Failed to create PDF dataset");

            GDALClose(poOutDS);
            GDALClose(poSrcDS);

            // Reopen and check properties
            GDALDataset* poCheckDS = static_cast<GDALDataset*>(GDALOpen(outputPath, GA_ReadOnly));
            Assert::IsNotNull(poCheckDS, L"Failed to reopen output PDF");

            //if (pdf_checksum_available())
            //{
            //    int checksum = poCheckDS->GetRasterBand(1)->Checksum();
            //    Assert::IsTrue(checksum != 0, L"PDF checksum is zero (unexpected)");
            //}

            double gt[6];
            CPLErr err = poCheckDS->GetGeoTransform(gt);
            Assert::AreEqual(CE_None, err, L"Failed to get geotransform");

            const char* srs = poCheckDS->GetProjectionRef();
            Assert::IsTrue(srs != nullptr && std::strlen(srs) > 0, L"SRS not found in PDF");

            GDALClose(poCheckDS);
        }

        TEST_METHOD(Test_CreateCopy_DPI300) {
            // Check if the PDF driver is available
            GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("PDF");
            if (!poDriver) {
                Logger::WriteMessage("PDF driver not available.");
                return;
            }

            // Load source dataset
            const char* srcPath = "/testdata/byte.tif";
            GDALDataset* poSrcDS = static_cast<GDALDataset*>(GDALOpen(srcPath, GA_ReadOnly));
            Assert::IsNotNull(poSrcDS, L"Source dataset could not be opened.");

            // Define output path and options
            const char* dstPath = "tmp/pdf_iso32000_dpi_300.pdf";
            const char* options[] = { "DPI=300", nullptr };

            // Create PDF copy
            GDALDataset* poDstDS = poDriver->CreateCopy(dstPath, poSrcDS, FALSE, const_cast<char**>(options), nullptr, nullptr);
            Assert::IsNotNull(poDstDS, L"CreateCopy failed.");
            GDALClose(poDstDS);
            GDALClose(poSrcDS);

            // Reopen for validation
            poDstDS = static_cast<GDALDataset*>(GDALOpen(dstPath, GA_ReadOnly));
            Assert::IsNotNull(poDstDS, L"Failed to reopen output PDF.");

            // Validate geo transform
            double geoTransform[6];
            CPLErr err = poDstDS->GetGeoTransform(geoTransform);
            Assert::AreEqual(CE_None, err, L"Failed to get GeoTransform");

            // Validate projection
            const char* pszSRS = poDstDS->GetProjectionRef();
            Assert::IsFalse(std::string(pszSRS).empty(), L"Projection string is empty");

            // Validate checksum
            //if (pdf_checksum_available()) {
            //    int cs = poDstDS->GetRasterBand(1)->Checksum();
            //    Assert::IsTrue(cs != 0, L"Checksum is zero, rendering likely failed");
            //}

            GDALClose(poDstDS);
        }

        TEST_METHOD(Test_CreateCopy_OGCBP_DPI300) {
            // Check if PDF driver is available
            GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("PDF");
            if (!poDriver) {
                Logger::WriteMessage("PDF driver not available.");
                return;
            }

            // Set config option
            CPLSetConfigOption("GDAL_PDF_OGC_BP_WRITE_WKT", "FALSE");

            // Open source dataset
            const char* srcPath = "../../testdata/byte.tif";
            GDALDataset* poSrcDS = static_cast<GDALDataset*>(GDALOpen(srcPath, GA_ReadOnly));
            Assert::IsNotNull(poSrcDS, L"Source dataset could not be opened.");

            // Create output
            const char* dstPath = "tmp/pdf_ogcbp_dpi_300.pdf";
            const char* options[] = { "GEO_ENCODING=OGC_BP", "DPI=300", nullptr };

            GDALDataset* poDstDS = poDriver->CreateCopy(dstPath, poSrcDS, FALSE, const_cast<char**>(options), nullptr, nullptr);
            Assert::IsNotNull(poDstDS, L"CreateCopy failed.");
            GDALClose(poDstDS);
            GDALClose(poSrcDS);

            // Reopen the written PDF for validation
            poDstDS = static_cast<GDALDataset*>(GDALOpen(dstPath, GA_ReadOnly));
            Assert::IsNotNull(poDstDS, L"Failed to reopen output PDF.");

            // Validate GeoTransform
            double geoTransform[6];
            CPLErr err = poDstDS->GetGeoTransform(geoTransform);
            Assert::AreEqual(CE_None, err, L"Failed to get GeoTransform");

            // Validate projection string is not empty
            const char* pszSRS = poDstDS->GetProjectionRef();
            Assert::IsFalse(std::string(pszSRS).empty(), L"Projection string is empty");

            // Check checksum
            //if (pdf_checksum_available()) {
            //    int cs = poDstDS->GetRasterBand(1)->Checksum();
            //    Assert::IsTrue(cs != 0, L"Checksum is zero");
            //}

            GDALClose(poDstDS);

            // Clear config option
            CPLSetConfigOption("GDAL_PDF_OGC_BP_WRITE_WKT", nullptr);
        }

        TEST_METHOD(Test_Pdf_OGCBP_LCC_Projection) {
            GDALAllRegister();

            GDALDriver* pdfDriver = GetGDALDriverManager()->GetDriverByName("PDF");
            GDALDriver* tiffDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
            Assert::IsNotNull(pdfDriver, L"PDF driver not found.");
            Assert::IsNotNull(tiffDriver, L"GTiff driver not found.");

            const char* tempTiffPath = "tmp/temp.tif";
            const char* outputPdfPath = "tmp/pdf_ogcbp_lcc.pdf";

            const char* wkt = R"(PROJCS["NAD83 / Utah North",
    GEOGCS["NAD83",
        DATUM["North_American_Datum_1983",
            SPHEROID["GRS 1980",6378137,298.257222101,
                AUTHORITY["EPSG","7019"]],
            TOWGS84[0,0,0,0,0,0,0]],
        PRIMEM["Greenwich",0],
        UNIT["degree",0.0174532925199433]],
    PROJECTION["Lambert_Conformal_Conic_2SP"],
    PARAMETER["standard_parallel_1",41.78333333333333],
    PARAMETER["standard_parallel_2",40.71666666666667],
    PARAMETER["latitude_of_origin",40.33333333333334],
    PARAMETER["central_meridian",-111.5],
    PARAMETER["false_easting",500000],
    PARAMETER["false_northing",1000000],
    UNIT["metre",1]])";

            // Create source GeoTIFF
            GDALDataset* poSrcDS = tiffDriver->Create(tempTiffPath, 1, 1, 1, GDT_Byte, nullptr);
            Assert::IsNotNull(poSrcDS, L"Failed to create temporary TIFF.");
            poSrcDS->SetProjection(wkt);
            double transform[6] = { 500000, 1, 0, 1000000, 0, -1 };
            poSrcDS->SetGeoTransform(transform);

            // Set config and Create PDF
            CPLSetConfigOption("GDAL_PDF_OGC_BP_WRITE_WKT", "FALSE");
            GDALDataset* poOutDS = pdfDriver->CreateCopy(outputPdfPath, poSrcDS, FALSE, nullptr, nullptr, nullptr);
            Assert::IsNotNull(poOutDS, L"Failed to create PDF.");

            // Get and compare WKT
            const char* outWkt = poOutDS->GetProjectionRef();
            OGRSpatialReference refSrc, refOut;
            refSrc.importFromWkt(wkt);
            refOut.importFromWkt(outWkt);

            Assert::IsTrue(refSrc.IsSame(&refOut) != 0, L"Output projection WKT does not match input.");

            // Cleanup
            GDALClose(poOutDS);
            GDALClose(poSrcDS);
            CPLSetConfigOption("GDAL_PDF_OGC_BP_WRITE_WKT", nullptr);
            pdfDriver->Delete(outputPdfPath);
            tiffDriver->Delete(tempTiffPath);
        }

        TEST_METHOD(Test_Pdf_No_Compression) {
            GDALAllRegister();

            GDALDriver* pdfDriver = GetGDALDriverManager()->GetDriverByName("PDF");
            GDALDriver* tiffDriver = GetGDALDriverManager()->GetDriverByName("GTiff");

            Assert::IsNotNull(pdfDriver, L"PDF driver not found.");
            Assert::IsNotNull(tiffDriver, L"GTiff driver not found.");

            const char* inputTiffPath = "../../testdata/byte.tif";
            const char* outputPdfPath = "tmp/pdf_no_compression.pdf";

            // Open source TIFF
            GDALDataset* poSrcDS = static_cast<GDALDataset*>(GDALOpen(inputTiffPath, GA_ReadOnly));
            Assert::IsNotNull(poSrcDS, L"Failed to open input TIFF.");

            // PDF creation options
            char* options[] = { const_cast<char*>("COMPRESS=NONE"), nullptr };

            // Create the PDF with no compression
            GDALDataset* poOutDS = pdfDriver->CreateCopy(outputPdfPath, poSrcDS, FALSE, options, nullptr, nullptr);
            Assert::IsNotNull(poOutDS, L"Failed to create PDF.");


            // Cleanup
            GDALClose(poOutDS);
            GDALClose(poSrcDS);
            pdfDriver->Delete(outputPdfPath);
        }

        TEST_METHOD(Test_CopyPDFToTIFF)
        {
            const char* input_pdf = "../../testdata/adobe_style_geospatial.pdf";
            const char* output_tiff = "tmp/test_output.tif";
            test_create_copy_and_verify(input_pdf, output_tiff);
        }

};
}
