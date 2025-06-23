#include "CppUnitTest.h"

#include <cstdlib>
#include <fstream>
#include <iostream>

#include <memory>
#include <string>
#include <stdexcept>

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

#include "gdal/gcore/gdal.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace gdalpdfunittest
{
    TEST_CLASS(Tests__GeoPDF_TransformProjection)
    {
        void assert_geo_transform(double expected_transform[6], double actual_transform[6])
        {
            Assert::IsTrue(
                actual_transform[0] == expected_transform[0]
                && actual_transform[1] == expected_transform[1]
                && actual_transform[2] == expected_transform[2]
                && actual_transform[3] == expected_transform[3]
                && actual_transform[4] == expected_transform[4]
                && actual_transform[5] == expected_transform[5],
                L"PDF GeoTransform is not as expected"
            );
        }

        void assert_geo_projection(const char* expected_projection, const char* actual_projection)
        {
            Assert::IsNotNull(actual_projection,
                L"PDF Projection is empty"
            );

            Assert::IsTrue(
                strcmp(actual_projection, expected_projection) == 0,
                L"PDF Projection is not as expected"
            );
        }

        void assert_geo_neatline(const char* expected_neatline, const char* actual_neatline)
        {
            Assert::IsNotNull(actual_neatline,
                L"PDF Neatline is empty"
            );

            Assert::IsTrue(
                strcmp(actual_neatline, expected_neatline) == 0,
                L"PDF Neatline is not as expected"
            );
        }

        void test_transform_and_projection(const char* filename,
            double expected_transform[6],
            const char* expected_projection,
            const char* expected_neatline)
        {
            GDALAllRegister();

            GDALDriverH driver = GDALGetDriverByName("PDF");
            Assert::IsNotNull(driver, L"PDF Driver not found");

            const char* has_raster = GDALGetMetadataItem(driver, GDAL_DCAP_RASTER, nullptr);
            Assert::IsTrue(std::string("YES")._Equal(has_raster));

            const char* is_pdfium = GDALGetMetadataItem(driver, "HAVE_PDFIUM", nullptr);
            Assert::IsTrue(std::string("YES")._Equal(is_pdfium));

            GDALDatasetH dataset = GDALOpen(filename, GDALAccess::GA_ReadOnly);

            Assert::IsNotNull(driver, L"PDF not open");

            double geo_transform[6];
            GDALGetGeoTransform(dataset, geo_transform);
            assert_geo_transform(expected_transform, geo_transform);

            if (expected_projection != 0)
            {
                const char* projection = nullptr;
                projection = GDALGetProjectionRef(dataset);
                assert_geo_projection(expected_projection, projection);
            }

            if (expected_neatline != 0)
            {
                const char* neatline = GDALGetMetadataItem(dataset, "NEATLINE", nullptr);
                assert_geo_neatline(expected_neatline, neatline);
            }

            GDALDestroyDriverManager();
        }

    public:
        TEST_METHOD(Test01_hambertfield)
        {
            double expected_transform[6] = {
                518377.09022965282,
                10.988837769018501,
                -0.0000000000000000,
                4467515.2003408950,
                0.0000000000000000,
                -10.988837769018501
            };

            const char* expected_projection =
                "PROJCS[\"UTM Zone 13, Northern Hemisphere\",GEOGCS[\"unknown\",DATUM[\"North_American_Datum_1983\",\
SPHEROID[\"GRS 1980\",6378137,298.257222101],TOWGS84[-0.991,1.9072,0.5129,0,0,0,0]],\
PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],\
PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-105],PARAMETER[\"scale_factor\",0.9996],\
PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"Meter\",1]]";

            const char* expected_neatline = "POLYGON ((\
532399.963276758 4451854.15467102,\
518364.98533805 4451854.15467102,\
518364.98533805 4467507.1077054,\
532399.963276758 4467507.1077054,\
532399.963276758 4451854.15467102))";

            test_transform_and_projection("../../testdata/hambertfield_geopdf.pdf", expected_transform, expected_projection, expected_neatline);
        }

        TEST_METHOD(Test02_Map_1_Ross_County_Ohio)
        {
            double expected_transform[6] = {
                539285.68121804390,
                0.28642004483287681,
                -0.10424837081662566,
                162647.66429865628,
                -0.10424837081635495,
                -0.28642004483287969
            };

            const char* expected_projection =
                "LOCAL_CS[\"State Plane Zone 3402 / NAD83\",UNIT[\"Meter\",1]]";

            const char* expected_neatline = "POLYGON ((\
540450.720776913 161072.466832451,\
538927.353252993 161626.927266999,\
539292.99615513 162631.52288419,\
540816.363679041 162077.062449633,\
540450.720776913 161072.466832451))";

            test_transform_and_projection("../../testdata/Map_1-Ross-County-Ohio.pdf",
                expected_transform, expected_projection, expected_neatline);
        }

        TEST_METHOD(Test03_NJ_Post_Sandy_mobile)
        {
            double expected_transform[6] = {
                185601.67286035881,
                0.93134124250833406,
                -0.0000000000000000,
                123870.50065849144,
                0.0000000000000000,
                -0.93134124250833406
            };

            const char* expected_projection =
                "LOCAL_CS[\"State Plane Zone 2900 / NAD83\",UNIT[\"Meter\",1]]";

            const char* expected_neatline = "POLYGON ((\
187081.577534317 121652.935251628,\
185629.105093322 121652.935251628,\
185629.105093322 123813.483892805,\
187081.577534317 123813.483892805,\
187081.577534317 121652.935251628))";

            test_transform_and_projection("../../testdata/NJ_Post_Sandy_mobile.pdf",
                expected_transform, expected_projection, expected_neatline);
        }

        TEST_METHOD(Test04_Land_Use_Change)
        {
            double expected_transform[6] = {
                192752.00000000000,
                20.000000000000000,
                0.0000000000000000,
                3773791.0000000000,
                0.0000000000000000,
                -20.000000000000000
            };

            const char* expected_projection =
                "PROJCS[\"UTM Zone 17, Northern Hemisphere\",GEOGCS[\"NAD27\",DATUM[\"North_American_Datum_1927\",\
SPHEROID[\"Clarke 1866\",6378206.4,294.9786982138982,AUTHORITY[\"EPSG\",\"7008\"]],AUTHORITY[\"EPSG\",\
\"6267\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,\
AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4267\"]],PROJECTION[\"Transverse_Mercator\"],\
PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-81],PARAMETER[\"scale_factor\",0.9996],\
PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"Meter\",1]]";

            const char* expected_neatline = "POLYGON ((\
192752.0 3768671.00011444,\
192752.0 3773791.00011444,\
197872.0 3773791.00011444,\
197872.0 3768671.00011444,\
192752.0 3768671.00011444))";

            test_transform_and_projection("../../testdata/Land-Use-Change.pdf",
                expected_transform, expected_projection, expected_neatline);
        }

        TEST_METHOD(Test05_adobe_style_geospatial)
        {
            double expected_transform[6] = {
                185601.67286035881,
                0.93134124250833406,
                -0.0000000000000000,
                123870.50065849144,
                0.0000000000000000,
                -0.93134124250833406
            };

            const char* expected_projection =
                "LOCAL_CS[\"State Plane Zone 2900 / NAD83\",UNIT[\"Meter\",1]]";

            const char* expected_neatline = "POLYGON (())";
            test_transform_and_projection("../../testdata/adobe_style_geospatial.pdf",
                expected_transform, expected_projection, expected_neatline);
        }

        TEST_METHOD(Test06_adobe_style_geospatial_with_xmp)
        {
            double expected_transform[6] = {
                185601.67286035881,
                0.93134124250833406,
                -0.0000000000000000,
                123870.50065849144,
                0.0000000000000000,
                -0.93134124250833406
            };

            const char* expected_projection =
                "LOCAL_CS[\"State Plane Zone 2900 / NAD83\",UNIT[\"Meter\",1]]";

            const char* expected_neatline = "POLYGON (())";
            test_transform_and_projection("../../testdata/adobe_style_geospatial_with_xmp.pdf",
                expected_transform, expected_projection, expected_neatline);
        }

        TEST_METHOD(Test07_test_pdf_composition_raster_georeferenced)
        {
            double expected_transform[6] = {
                185601.67286035881,
                0.93134124250833406,
                -0.0000000000000000,
                123870.50065849144,
                0.0000000000000000,
                -0.93134124250833406
            };

            const char* expected_projection =
                "LOCAL_CS[\"State Plane Zone 2900 / NAD83\",UNIT[\"Meter\",1]]";

            const char* expected_neatline = "POLYGON (())";
            test_transform_and_projection("../../testdata/test_pdf_composition_raster_georeferenced.pdf",
                expected_transform, expected_projection, expected_neatline);
        }

        TEST_METHOD(Test08_test_pdf_composition_raster_georeferenced_libpng_1_6_40)
        {
            double expected_transform[6] = {
                185601.67286035881,
                0.93134124250833406,
                -0.0000000000000000,
                123870.50065849144,
                0.0000000000000000,
                -0.93134124250833406
            };

            const char* expected_projection =
                "LOCAL_CS[\"State Plane Zone 2900 / NAD83\",UNIT[\"Meter\",1]]";

            const char* expected_neatline = "POLYGON (())";
            test_transform_and_projection("../../testdata/test_pdf_composition_raster_georeferenced_libpng_1_6_40.pdf",
                expected_transform, expected_projection, expected_neatline);
        }

        TEST_METHOD(Test09_test_pdf_composition_raster_tiled_blending)
        {
            double expected_transform[6] = {
                185601.67286035881,
                0.93134124250833406,
                -0.0000000000000000,
                123870.50065849144,
                0.0000000000000000,
                -0.93134124250833406
            };

            const char* expected_projection =
                "LOCAL_CS[\"State Plane Zone 2900 / NAD83\",UNIT[\"Meter\",1]]";

            const char* expected_neatline = "POLYGON (())";
            test_transform_and_projection("../../testdata/test_pdf_composition_raster_tiled_blending.pdf",
                expected_transform, expected_projection, expected_neatline);
        }

        TEST_METHOD(Test10_test_pdf_composition_raster_tiled_blending_libpng_1_6_40)
        {
            double expected_transform[6] = {
                185601.67286035881,
                0.93134124250833406,
                -0.0000000000000000,
                123870.50065849144,
                0.0000000000000000,
                -0.93134124250833406
            };

            const char* expected_projection =
                "LOCAL_CS[\"State Plane Zone 2900 / NAD83\",UNIT[\"Meter\",1]]";

            const char* expected_neatline = "POLYGON (())";
            test_transform_and_projection("../../testdata/test_pdf_composition_raster_tiled_blending_libpng_1_6_40.pdf",
                expected_transform, expected_projection, expected_neatline);
        }
    };

    TEST_CLASS(Tests_gdal_extras) {

        TEST_METHOD(Test_ReadMetadataFields)
        {
            GDALAllRegister();
            const char* filename = "../../testdata/hambertfield_geopdf.pdf";
            GDALDatasetH dataset = GDALOpen(filename, GA_ReadOnly);
            Assert::IsNotNull(dataset);

            const char* author = GDALGetMetadataItem(dataset, "AUTHOR", NULL);
            const char* creator = GDALGetMetadataItem(dataset, "CREATOR", NULL);

            if (author) Logger::WriteMessage((std::string("Author: ") + author).c_str());
            if (creator) Logger::WriteMessage((std::string("Creator: ") + creator).c_str());

            GDALClose(dataset);
        }

        TEST_METHOD(Test_CheckGeoPDFLayers)
        {
            GDALAllRegister();

            const char* filepath = "../../testdata/hambertfield_geopdf.pdf";

            GDALDataset* poDS = static_cast<GDALDataset*>(GDALOpenEx(
                filepath,
                GDAL_OF_VECTOR | GDAL_OF_READONLY,
                nullptr, nullptr, nullptr
            ));

            Assert::IsNotNull(poDS, L"Failed to open GeoPDF as vector");

            int layerCount = poDS->GetLayerCount();
            Logger::WriteMessage(string_format("Total Layers Found: %d\n", layerCount).c_str());
            Assert::IsTrue(layerCount > 0, L"No layers found in GeoPDF!");

            for (int i = 0; i < layerCount; ++i)
            {
                OGRLayer* poLayer = poDS->GetLayer(i);
                Assert::IsNotNull(static_cast<void*>(poLayer), L"Layer is null");

                const char* layerName = poLayer->GetName();
                Logger::WriteMessage(string_format("Layer %d: %s\n", i, layerName).c_str());
            }

            GDALClose(poDS);
        }

        TEST_METHOD(Test_CheckGCPsIfPresent)
        {
            GDALAllRegister();
            const char* filename = "../../testdata/Map_1-Ross-County-Ohio.pdf";
            GDALDatasetH dataset = GDALOpen(filename, GA_ReadOnly);
            Assert::IsNotNull(dataset);

            int gcpCount = GDALGetGCPCount(dataset);
            if (gcpCount > 0) {
                const GDAL_GCP* gcps = GDALGetGCPs(dataset);
                Assert::IsNotNull(gcps);
            }
            GDALClose(dataset);
        }

    };

    // todo: extract test class into new file
    TEST_CLASS(Tests__gdal_translate_exe)
    {
        void run_gdal_translate(const char* filename)
        {
            std::string pdf_filename(filename);

            // todo: convert to full path where gdal_translate.exe is output after build
            std::string gdal_translate_exe("gdal_translate.exe");

            // https://imagemagick.org/script/download.php
            std::string magick_exe("magick.exe");

            // todo: make dir, and generate all test artifacts into it
            std::string translate_log = string_format("%s.translate.log", filename);
            std::string magick_log = string_format("%s.magick.log", filename);

            const char* img_format = "png";
            std::string expected_image = string_format("%s-0.%s", pdf_filename.c_str(), img_format);
            std::string actual_image = string_format("%s-1.%s", pdf_filename.c_str(), img_format);
            std::string diff_image = string_format("%s-2.%s", pdf_filename.c_str(), img_format);

            std::string cmd_translate = gdal_translate_exe + " " + pdf_filename + " " + actual_image + " >" + translate_log;
            std::system(cmd_translate.c_str());
            // std::cout << std::ifstream(translate_log.c_str()).rdbuf();

            std::string cmd_magick = string_format("%s %s %s -compare -metric RMSE -verbose -fuzz 99 -format \"%[distortion]\" %s > %s 2>&1",
                magick_exe.c_str(),
                expected_image.c_str(),
                actual_image.c_str(),
                diff_image.c_str(),
                magick_log.c_str()
            );

            // todo: here exit_code would be nice to check too
            std::system(cmd_magick.c_str());

            // todo: parse generated *.aux.xml
            // todo: check output of magick to see if diff was successful
        }

    public:
        TEST_METHOD(Test01_hambertfield)
        {
            run_gdal_translate("../../testdata/hambertfield_geopdf.pdf");
        }
    };
}
