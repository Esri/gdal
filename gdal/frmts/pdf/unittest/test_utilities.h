#pragma once

#include <string>
#include "cpl_error.h"

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

namespace Microsoft {
    namespace VisualStudio {
        namespace CppUnitTestFramework {

            template <>
            std::wstring ToString<CPLErr>(const CPLErr& err)
            {
                switch (err)
                {
                case CE_None: return L"CE_None";
                case CE_Debug: return L"CE_Debug";
                case CE_Warning: return L"CE_Warning";
                case CE_Failure: return L"CE_Failure";
                case CE_Fatal: return L"CE_Fatal";
                default: return L"Unknown CPLErr";
                }
            }

        } // namespace CppUnitTestFramework
    } // namespace VisualStudio
} // namespace Microsoft


bool file_exists(const char* filename);
bool download_file(const std::string& url, const std::string& local_path);
bool pdf_is_pdfium();
bool pdf_is_poppler();
bool pdf_checksum_available();
void test_create_copy_and_verify(const char* pszSource, const char* pszTarget);
void test_create_copy_in_same_directory(const char* pszSource, const char* pszOutputFilename);



