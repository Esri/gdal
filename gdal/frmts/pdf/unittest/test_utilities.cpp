#include "CppUnitTest.h"

#include <cstdlib>
#include <fstream>
#include <iostream>

#include <memory>
#include <string>
#include <stdexcept>
#include <chrono>
#include "test_utilities.h"

#include <windows.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")

#include "gdal/gdal.h"
#include "gdal/gdal_priv.h"
#include "ogr/ogrsf_frmts.h"
#include <filesystem>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

bool download_file(const std::string& url, const std::string& local_path) {
    std::wstring w_url(url.begin(), url.end());
    std::wstring w_path(local_path.begin(), local_path.end());

    HRESULT hr = URLDownloadToFileW(
        nullptr,
        w_url.c_str(),
        w_path.c_str(),
        0,
        nullptr
    );

    return SUCCEEDED(hr);
}

bool file_exists(const char* filename) {
    std::ifstream file(filename);
    return file.good();
}

bool pdf_is_pdfium()
{
    // Get PDF driver
    GDALDriverH driver = GDALGetDriverByName("PDF");
    if (!driver) return false;

    // Check GDAL_PDF_LIB config value
    const char* val = CPLGetConfigOption("GDAL_PDF_LIB", "PDFIUM");
    if (val == nullptr || std::string(val) != "PDFIUM")
        return false;

    // Check driver metadata for HAVE_PDFIUM
    const char* has_pdfium = GDALGetMetadataItem(driver, "HAVE_PDFIUM", nullptr);
    return has_pdfium != nullptr;
}

bool pdf_is_poppler()
{
    // Get PDF driver
    GDALDriverH driver = GDALGetDriverByName("PDF");
    if (!driver) return false;

    // Check GDAL_PDF_LIB config value
    const char* val = CPLGetConfigOption("GDAL_PDF_LIB", "POPPLER");
    if (val == nullptr || std::string(val) != "POPPLER")
        return false;

    // Check driver metadata for HAVE_POPPLER
    const char* has_poppler = GDALGetMetadataItem(driver, "HAVE_POPPLER", nullptr);
    if (!has_poppler)
        return false;

    return !pdf_is_pdfium();
}

bool pdf_checksum_available()
{
    static bool is_initialized = false;
    static bool cached_result = false;

    if (is_initialized)
        return cached_result;

    is_initialized = true;

    if (pdf_is_poppler() || pdf_is_pdfium())
    {
        cached_result = true;
        return true;
    }

    std::string command = "pdftoppm -v 2>&1";
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe)
    {
        std::cerr << "Failed to run pdftoppm command." << std::endl;
        cached_result = false;
        return false;
    }

    char buffer[256];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        result += buffer;
    }
    _pclose(pipe);

    if (result.find("pdftoppm version") == 0)
    {
        cached_result = true;
        return true;
    }
    else
    {
        std::cerr << "Cannot compute checksum due to missing pdftoppm" << std::endl;
        std::cerr << result << std::endl;
        cached_result = false;
        return false;
    }
}

void test_create_copy_and_verify(const char* pszSource, const char* pszTarget)
{
    GDALAllRegister();

    // Ensure source exists
    std::ifstream infile(pszSource);
    Assert::IsTrue(infile.good(), L"Input file does not exist");

    // Create output directory
    std::filesystem::create_directories("tmp");

    const char* const open_options[] = { "LAYERS=ALL", nullptr };

    GDALDatasetH in_dataset = GDALOpenEx(pszSource, GDAL_OF_RASTER, nullptr, open_options, nullptr);
    Assert::IsNotNull(in_dataset, L"Failed to open source dataset");

    GDALDriverH dst_driver = GDALGetDriverByName("GTIFF");
    Assert::IsNotNull(dst_driver, L"GTIFF driver not found");

    GDALDatasetH out_dataset = GDALCreateCopy(dst_driver, pszTarget, in_dataset, 0, nullptr, nullptr, nullptr);
    Assert::IsNotNull(out_dataset, L"Failed to create output copy");

    GDALFlushCache(out_dataset);
    GDALClose(out_dataset);
    GDALClose(in_dataset);

    GDALDatasetH verify_dataset = GDALOpen(pszTarget, GA_ReadOnly);
    Assert::IsNotNull(verify_dataset, L"Copied output file could not be opened");

    GDALClose(verify_dataset);

    GDALDestroyDriverManager();
}

void test_create_copy_in_same_directory(const char* pszSource, const char* pszOutputFilename)
{
    GDALAllRegister();

    // Ensure source exists
    std::ifstream infile(pszSource);
    Assert::IsTrue(infile.good(), L"Input file does not exist");

    // Extract parent directory from source
    std::filesystem::path sourcePath(pszSource);
    std::filesystem::path parentDir = sourcePath.parent_path();
    std::filesystem::create_directories(parentDir);  // Ensure dir exists

    // Construct full output path in same directory
    std::filesystem::path outputPath = parentDir / pszOutputFilename;
    std::string outputPathStr = outputPath.string();

    const char* const open_options[] = { "LAYERS=ALL", nullptr };

    GDALDatasetH in_dataset = GDALOpenEx(pszSource, GDAL_OF_RASTER, nullptr, open_options, nullptr);
    Assert::IsNotNull(in_dataset, L"Failed to open source dataset");

    GDALDriverH dst_driver = GDALGetDriverByName("GTIFF");
    Assert::IsNotNull(dst_driver, L"GTIFF driver not found");

    GDALDatasetH out_dataset = GDALCreateCopy(dst_driver, outputPathStr.c_str(), in_dataset, 0, nullptr, nullptr, nullptr);
    Assert::IsNotNull(out_dataset, L"Failed to create output copy");

    GDALFlushCache(out_dataset);
    GDALClose(out_dataset);
    GDALClose(in_dataset);

    GDALDatasetH verify_dataset = GDALOpen(outputPathStr.c_str(), GA_ReadOnly);
    Assert::IsNotNull(verify_dataset, L"Copied output file could not be opened");

    GDALClose(verify_dataset);
    GDALDestroyDriverManager();
}

