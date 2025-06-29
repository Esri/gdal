/******************************************************************************
 *
 * Project:  WCS Client Driver
 * Purpose:  Implementation of utilities.
 * Author:   Ari Jolma <ari dot jolma at gmail dot com>
 *
 ******************************************************************************
 * Copyright (c) 2006, Frank Warmerdam
 * Copyright (c) 2008-2013, Even Rouault <even dot rouault at spatialys.com>
 * Copyright (c) 2017, Ari Jolma
 * Copyright (c) 2017, Finnish Environment Institute
 *
 * SPDX-License-Identifier: MIT
 ****************************************************************************/

#include "ogr_spatialref.h"
#include "wcsutils.h"
#include <algorithm>

#define DIGITS "0123456789"

namespace WCSUtils
{

void Swap(double &a, double &b)
{
    double tmp = a;
    a = b;
    b = tmp;
}

std::string URLEncode(const std::string &str)
{
    char *pszEncoded = CPLEscapeString(str.c_str(), -1, CPLES_URL);
    std::string str2 = pszEncoded;
    CPLFree(pszEncoded);
    return str2;
}

std::string URLRemoveKey(const char *url, const std::string &key)
{
    CPLString retval = url;
    const std::string key_is = key + "=";
    while (true)
    {
        size_t pos = retval.ifind(key_is);
        if (pos != std::string::npos)
        {
            size_t end = retval.find("&", pos);
            retval.erase(pos, end - pos + 1);
        }
        else
        {
            break;
        }
    }
    if (!retval.empty() && retval.back() == '&')
    {
        retval.pop_back();
    }
    std::string retValAsStdString = std::move(retval);
    return retValAsStdString;
}

std::vector<std::string> &SwapFirstTwo(std::vector<std::string> &array)
{
    if (array.size() >= 2)
    {
        std::swap(array[0], array[1]);
    }
    return array;
}

std::vector<std::string> Split(const char *value, const char *delim,
                               bool swap_the_first_two)
{
    std::vector<std::string> array;
    char **tokens = CSLTokenizeString2(
        value, delim,
        CSLT_STRIPLEADSPACES | CSLT_STRIPENDSPACES | CSLT_HONOURSTRINGS);
    int n = CSLCount(tokens);
    for (int i = 0; i < n; ++i)
    {
        array.push_back(tokens[i]);
    }
    CSLDestroy(tokens);
    if (swap_the_first_two && array.size() >= 2)
    {
        return SwapFirstTwo(array);
    }
    return array;
}

std::string Join(const std::vector<std::string> &array, const char *delim,
                 bool swap_the_first_two)
{
    std::string str;
    const auto arraySize = array.size();
    for (unsigned int i = 0; i < arraySize; ++i)
    {
        if (i > 0)
        {
            str += delim;
        }
        if (swap_the_first_two)
        {
            if (i == 0 && arraySize >= 2)
            {
                str += array[1];
            }
            else if (i == 1)
            {
                str += array[0];
            }
        }
        else
        {
            str += array[i];
        }
    }
    return str;
}

std::vector<int> Ilist(const std::vector<std::string> &array, unsigned int from,
                       size_t count)
{
    std::vector<int> retval;
    for (unsigned int i = from; i < array.size() && i < from + count; ++i)
    {
        retval.push_back(atoi(array[i].c_str()));
    }
    return retval;
}

std::vector<double> Flist(const std::vector<std::string> &array,
                          unsigned int from, size_t count)
{
    std::vector<double> retval;
    for (unsigned int i = from; i < array.size() && i < from + count; ++i)
    {
        retval.push_back(CPLAtof(array[i].c_str()));
    }
    return retval;
}

int IndexOf(const std::string &str, const std::vector<std::string> &array)
{
    int index = -1;
    for (unsigned int i = 0; i < array.size(); ++i)
    {
        if (array[i] == str)
        {
            index = i;
            break;
        }
    }
    return index;
}

int IndexOf(int i, const std::vector<int> &array)
{
    int index = -1;
    for (unsigned int j = 0; j < array.size(); ++j)
    {
        if (array[j] == i)
        {
            index = j;
            break;
        }
    }
    return index;
}

std::vector<int> IndexOf(const std::vector<std::string> &strs,
                         const std::vector<std::string> &array)
{
    std::vector<int> retval;
    for (unsigned int i = 0; i < strs.size(); ++i)
    {
        retval.push_back(IndexOf(strs[i], array));
    }
    return retval;
}

int IndexOf(const std::string &key,
            const std::vector<std::vector<std::string>> &kvps)
{
    int index = -1;
    for (unsigned int i = 0; i < kvps.size(); ++i)
    {
        if (kvps[i].size() > 1 && key == kvps[i][0])
        {
            index = i;
            break;
        }
    }
    return index;
}

bool Contains(const std::vector<int> &array, int value)
{
    for (unsigned int i = 0; i < array.size(); ++i)
    {
        if (array[i] == value)
        {
            return true;
        }
    }
    return false;
}

std::string FromParenthesis(const std::string &s)
{
    size_t beg = s.find_first_of("(");
    size_t end = s.find_last_of(")");
    if (beg == std::string::npos || end == std::string::npos)
    {
        return "";
    }
    return s.substr(beg + 1, end - beg - 1);
}

std::vector<std::string>
ParseSubset(const std::vector<std::string> &subset_array,
            const std::string &dim)
{
    // array is SUBSET defs, a SUBSET def is dim[,crs](low[,high])
    std::vector<std::string> retval;
    unsigned int i;
    std::string params;
    for (i = 0; i < subset_array.size(); ++i)
    {
        params = subset_array[i];
        size_t pos = params.find(dim + "(");
        if (pos != std::string::npos)
        {
            retval.push_back("");  // crs
            break;
        }
        pos = params.find(dim + ",");
        if (pos != std::string::npos)
        {
            params.erase(0, pos + 1);
            pos = params.find("(");
            retval.push_back(params.substr(0, pos - 1));
            break;
        }
    }
    if (retval.size() > 0)
    {
        std::vector<std::string> params_array =
            Split(FromParenthesis(params).c_str(), ",");
        retval.push_back(params_array[0]);
        if (params_array.size() > 1)
        {
            retval.push_back(params_array[1]);
        }
        else
        {
            retval.push_back("");
        }
    }
    return retval;
}

/* -------------------------------------------------------------------- */
/*      FileIsReadable                                                  */
/* -------------------------------------------------------------------- */

bool FileIsReadable(const std::string &filename)
{
    VSILFILE *file = VSIFOpenL(filename.c_str(), "r");
    if (file)
    {
        VSIFCloseL(file);
        return true;
    }
    return false;
}

std::string RemoveExt(const std::string &filename)
{
    size_t pos = filename.find_last_of(".");
    if (pos != std::string::npos)
    {
        return filename.substr(0, pos);
    }
    return filename;
}

/* -------------------------------------------------------------------- */
/*      MakeDir                                                         */
/* -------------------------------------------------------------------- */

bool MakeDir(const std::string &dirname)
{
    VSIStatBufL stat;
    if (VSIStatL(dirname.c_str(), &stat) != 0)
    {
        std::string parent = CPLGetDirnameSafe(dirname.c_str());
        if (!parent.empty() && parent != ".")
        {
            if (!MakeDir(parent))
            {
                return false;
            }
        }
        return VSIMkdir(dirname.c_str(), 0755) == 0;
    }
    return true;
}

/************************************************************************/
/*                       SearchChildWithValue()                         */
/************************************************************************/

CPLXMLNode *SearchChildWithValue(CPLXMLNode *node, const char *path,
                                 const char *value)
{
    if (node == nullptr)
    {
        return nullptr;
    }
    for (CPLXMLNode *child = node->psChild; child != nullptr;
         child = child->psNext)
    {
        if (EQUAL(CPLGetXMLValue(child, path, ""), value))
        {
            return child;
        }
    }
    return nullptr;
}

bool CPLGetXMLBoolean(CPLXMLNode *poRoot, const char *pszPath)
{
    // returns true if path exists and does not contain untrue value
    poRoot = CPLGetXMLNode(poRoot, pszPath);
    if (poRoot == nullptr)
    {
        return false;
    }
    return CPLTestBool(CPLGetXMLValue(poRoot, nullptr, ""));
}

bool CPLUpdateXML(CPLXMLNode *poRoot, const char *pszPath,
                  const char *new_value)
{
    std::string old_value = CPLGetXMLValue(poRoot, pszPath, "");
    if (new_value != old_value)
    {
        CPLSetXMLValue(poRoot, pszPath, new_value);
        return true;
    }
    return false;
}

/* -------------------------------------------------------------------- */
/*      XMLCopyMetadata                                                 */
/*      Copy child node 'key' into metadata as MDI element.             */
/* -------------------------------------------------------------------- */

void XMLCopyMetadata(CPLXMLNode *parent, CPLXMLNode *metadata,
                     const std::string &key)
{
    CPLXMLNode *node = CPLGetXMLNode(parent, key.c_str());
    if (node)
    {
        CPLAddXMLAttributeAndValue(
            CPLCreateXMLElementAndValue(metadata, "MDI",
                                        CPLGetXMLValue(node, nullptr, "")),
            "key", key.c_str());
    }
}

/* -------------------------------------------------------------------- */
/*      SetupCache                                                      */
/*      Cache is a directory                                            */
/*      The file db is the cache index with lines of unique_key=URL     */
/* -------------------------------------------------------------------- */

bool SetupCache(std::string &cache, bool clear)
{
    if (cache == "")
    {
#ifdef _WIN32
        const char *home = CPLGetConfigOption("USERPROFILE", nullptr);
#else
        const char *home = CPLGetConfigOption("HOME", nullptr);
#endif
        if (home)
        {
            cache = CPLFormFilenameSafe(home, ".gdal", nullptr);
        }
        else
        {
            const char *dir = CPLGetConfigOption("CPL_TMPDIR", nullptr);
            if (!dir)
                dir = CPLGetConfigOption("TMPDIR", nullptr);
            if (!dir)
                dir = CPLGetConfigOption("TEMP", nullptr);
            const char *username = CPLGetConfigOption("USERNAME", nullptr);
            if (!username)
                username = CPLGetConfigOption("USER", nullptr);
            if (dir && username)
            {
                std::string subdir = ".gdal_";
                subdir += username;
                cache = CPLFormFilenameSafe(dir, subdir.c_str(), nullptr);
            }
        }
        cache = CPLFormFilenameSafe(cache.c_str(), "wcs_cache", nullptr);
    }
    if (!MakeDir(cache))
    {
        return false;
    }
    if (clear)
    {
        char **folder = VSIReadDir(cache.c_str());
        int size = folder ? CSLCount(folder) : 0;
        for (int i = 0; i < size; i++)
        {
            if (folder[i][0] == '.')
            {
                continue;
            }
            const std::string filepath =
                CPLFormFilenameSafe(cache.c_str(), folder[i], nullptr);
            CPL_IGNORE_RET_VAL(VSIUnlink(filepath.c_str()));
        }
        CSLDestroy(folder);
    }
    // make sure the index exists and is writable
    const std::string db = CPLFormFilenameSafe(cache.c_str(), "db", nullptr);
    VSILFILE *f = VSIFOpenL(db.c_str(), "r");
    if (f)
    {
        VSIFCloseL(f);
    }
    else
    {
        f = VSIFOpenL(db.c_str(), "w");
        if (f)
        {
            VSIFCloseL(f);
        }
        else
        {
            CPLError(CE_Failure, CPLE_FileIO, "Can't open file '%s': %i\n",
                     db.c_str(), errno);
            return false;
        }
    }
    srand((unsigned int)time(
        nullptr));  // not to have the same names in the cache
    return true;
}

static bool CompareStrings(const std::string &a, const std::string &b)
{
    return a.compare(b) < 0;
}

std::vector<std::string> ReadCache(const std::string &cache)
{
    std::vector<std::string> contents;
    const std::string db = CPLFormFilenameSafe(cache.c_str(), "db", nullptr);
    char **data = CSLLoad(db.c_str());
    if (data)
    {
        for (int i = 0; data[i]; ++i)
        {
            char *val = strchr(data[i], '=');
            if (val != nullptr && *val == '=')
            {
                val += 1;
                if (strcmp(val, "bar") != 0)
                {
                    contents.push_back(val);
                }
            }
        }
        CSLDestroy(data);
    }
    std::sort(contents.begin(), contents.end(), CompareStrings);
    return contents;
}

/* -------------------------------------------------------------------- */
/*      DeleteEntryFromCache                                            */
/*      Examines the 'db' file in the cache, which contains             */
/*      unique key=value pairs, one per line. This function             */
/*      deletes pairs based on the given key and/or value.              */
/*      If key or value is empty it is not considered.                  */
/*      The key is taken as a basename of a file in the cache           */
/*      and all files with the basename is deleted.                     */
/* -------------------------------------------------------------------- */

bool DeleteEntryFromCache(const std::string &cache, const std::string &key,
                          const std::string &value)
{
    // Depending on which one of key and value is not "" delete the relevant
    // entry.
    const std::string db = CPLFormFilenameSafe(cache.c_str(), "db", nullptr);
    char **data =
        CSLLoad(db.c_str());  // returns NULL in error and for empty files
    char **data2 = CSLAddNameValue(nullptr, "foo", "bar");
    std::string filename = "";
    if (data)
    {
        for (int i = 0; data[i]; ++i)
        {
            char *val = strchr(data[i], '=');
            if (val != nullptr && *val == '=')
            {
                *val = '\0';
                val = val + 1;
                if ((key != "" && key == data[i]) ||
                    (value != "" && value == val) ||
                    (strcmp(data[i], "foo") == 0))
                {
                    if (key != "")
                    {
                        filename = data[i];
                    }
                    else if (value != "")
                    {
                        filename = data[i];
                    }
                    continue;
                }
                data2 = CSLAddNameValue(data2, data[i], val);
            }
        }
        CSLDestroy(data);
    }
    CSLSave(data2, db.c_str());  // returns 0 in error and for empty arrays
    CSLDestroy(data2);
    if (filename != "")
    {
        char **folder = VSIReadDir(cache.c_str());
        int size = folder ? CSLCount(folder) : 0;
        for (int i = 0; i < size; i++)
        {
            if (folder[i][0] == '.')
            {
                continue;
            }
            std::string name = folder[i];
            if (name.find(filename) != std::string::npos)
            {
                const std::string filepath =
                    CPLFormFilenameSafe(cache.c_str(), name.c_str(), nullptr);
                if (VSIUnlink(filepath.c_str()) == -1)
                {
                    // error but can't do much, raise a warning?
                }
            }
        }
        CSLDestroy(folder);
    }
    return true;
}

/* -------------------------------------------------------------------- */
/*      SearchCache                                                     */
/*      The key,value pairs in the cache index file 'db' is searched    */
/*      for the first pair where the value is the given url. If one     */
/*      is found, the filename is formed from the cache directory name, */
/*      the key, and the ext.                                           */
/* -------------------------------------------------------------------- */

CPLErr SearchCache(const std::string &cache, const std::string &url,
                   std::string &filename, const std::string &ext, bool &found)
{
    found = false;
    const std::string db = CPLFormFilenameSafe(cache.c_str(), "db", nullptr);
    VSILFILE *f = VSIFOpenL(db.c_str(), "r");
    if (!f)
    {
        CPLError(CE_Failure, CPLE_FileIO, "Can't open file '%s': %i\n",
                 db.c_str(), errno);
        return CE_Failure;
    }
    while (const char *line = CPLReadLineL(f))
    {
        char *value = strchr((char *)line, '=');
        if (value == nullptr || *value != '=')
        {
            continue;
        }
        *value = '\0';
        if (url == (value + 1))
        {
            filename = line;
            found = true;
            break;
        }
    }
    VSIFCloseL(f);
    if (found)
    {
        filename = CPLFormFilenameSafe(cache.c_str(), (filename + ext).c_str(),
                                       nullptr);
        found = FileIsReadable(filename);
        // if not readable, we should delete the entry
    }
    return CE_None;
}

/* -------------------------------------------------------------------- */
/*      AddEntryToCache                                                 */
/*      A new unique key is created into the database based on the      */
/*      filename replacing X with a random ascii character.             */
/*      The returned filename is a path formed from the cache directory */
/*      name, the filename, and the ext.                                */
/* -------------------------------------------------------------------- */

CPLErr AddEntryToCache(const std::string &cache, const std::string &url,
                       std::string &filename, const std::string &ext)
{
    // todo: check for lock and do something if locked(?)
    // todo: lock the cache
    // assuming the url is not in the cache
    const std::string store = filename;
    const std::string db = CPLFormFilenameSafe(cache.c_str(), "db", nullptr);
    VSILFILE *f = VSIFOpenL(db.c_str(), "a");
    if (!f)
    {
        CPLError(CE_Failure, CPLE_FileIO, "Can't open file '%s': %i\n",
                 db.c_str(), errno);
        return CE_Failure;
    }

    // create a new file into the cache using filename as template
    std::string path = "";
    VSIStatBufL stat;
    do
    {
        filename = store;
        static const char chars[] =
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        for (size_t i = 0; i < filename.length(); ++i)
        {
            if (filename.at(i) == 'X')
            {
#ifndef __COVERITY__
                filename.replace(i, 1, 1, chars[rand() % (sizeof(chars) - 1)]);
#else
                filename.replace(i, 1, 1, chars[i % (sizeof(chars) - 1)]);
#endif
            }
        }
        // replace X with random character from a-zA-Z
        path = CPLFormFilenameSafe(cache.c_str(), (filename + ext).c_str(),
                                   nullptr);
    } while (VSIStatExL(path.c_str(), &stat, VSI_STAT_EXISTS_FLAG) == 0);
    VSILFILE *f2 = VSIFOpenL(path.c_str(), "w");
    if (f2)
    {
        VSIFCloseL(f2);
    }

    std::string entry =
        filename + "=" + url + "\n";  // '=' for compatibility with CSL
    VSIFWriteL(entry.c_str(), sizeof(char), entry.size(), f);
    VSIFCloseL(f);

    filename = std::move(path);
    return CE_None;
}

// steps into element 'from' and adds values of elements 'keys' into the
// metadata 'path' is the key that is used for metadata and it is appended with
// 'from' path may be later used to get metadata from elements below 'from' so
// it is returned in the appended form
CPLXMLNode *AddSimpleMetaData(char ***metadata, CPLXMLNode *node,
                              std::string &path, const std::string &from,
                              const std::vector<std::string> &keys)
{
    CPLXMLNode *node2 = CPLGetXMLNode(node, from.c_str());
    if (node2)
    {
        path = path + from + ".";
        for (unsigned int i = 0; i < keys.size(); i++)
        {
            CPLXMLNode *node3 = CPLGetXMLNode(node2, keys[i].c_str());
            if (node3)
            {
                const std::string name = path + keys[i];
                CPLString value = CPLGetXMLValue(node3, nullptr, "");
                value.Trim();
                *metadata =
                    CSLSetNameValue(*metadata, name.c_str(), value.c_str());
            }
        }
    }
    return node2;
}

std::string GetKeywords(CPLXMLNode *root, const std::string &path,
                        const std::string &kw)
{
    std::string words = "";
    CPLXMLNode *keywords =
        (path != "") ? CPLGetXMLNode(root, path.c_str()) : root;
    if (keywords)
    {
        std::vector<unsigned int> epsg_codes;
        for (CPLXMLNode *node = keywords->psChild; node != nullptr;
             node = node->psNext)
        {
            if (node->eType != CXT_Element)
            {
                continue;
            }
            if (kw == node->pszValue)
            {
                CPLString word = CPLGetXMLValue(node, nullptr, "");
                word.Trim();

                // crs, replace "http://www.opengis.net/def/crs/EPSG/0/"
                // or "urn:ogc:def:crs:EPSG::" with EPSG:
                const char *const epsg[] = {
                    "http://www.opengis.net/def/crs/EPSG/0/",
                    "urn:ogc:def:crs:EPSG::"};
                for (unsigned int i = 0; i < CPL_ARRAYSIZE(epsg); i++)
                {
                    size_t pos = word.find(epsg[i]);
                    if (pos == 0)
                    {
                        std::string code =
                            word.substr(strlen(epsg[i]), std::string::npos);
                        if (code.find_first_not_of(DIGITS) == std::string::npos)
                        {
                            epsg_codes.push_back(atoi(code.c_str()));
                            continue;
                        }
                    }
                }

                // profiles, remove http://www.opengis.net/spec/
                // interpolation, remove
                // http://www.opengis.net/def/interpolation/OGC/1/

                const char *const spec[] = {
                    "http://www.opengis.net/spec/",
                    "http://www.opengis.net/def/interpolation/OGC/1/"};
                for (unsigned int i = 0; i < CPL_ARRAYSIZE(spec); i++)
                {
                    size_t pos = word.find(spec[i]);
                    if (pos != std::string::npos)
                    {
                        word.erase(pos, strlen(spec[i]));
                    }
                }

                if (words != "")
                {
                    words += ",";
                }
                words += word;
            }
        }
        if (epsg_codes.size() > 0)
        {
            std::string codes;
            std::sort(epsg_codes.begin(), epsg_codes.end());
            unsigned int pajazzo = 0, i = 0, a = 0, b = 0;
            while (1)
            {
                // cppcheck-suppress containerOutOfBounds
                unsigned int c = i < epsg_codes.size() ? epsg_codes[i] : 0;
                if (pajazzo == 1)
                {
                    if (c > a + 1)
                    {
                        if (codes != "")
                        {
                            codes += ",";
                        }
                        codes += CPLString().Printf("%i", a);
                        a = c;
                    }
                    else if (c >= a)
                    {
                        b = c;
                        pajazzo = 2;
                    }
                }
                else if (pajazzo == 2)
                {
                    if (c > b + 1)
                    {
                        if (codes != "")
                        {
                            codes += ",";
                        }
                        codes += CPLString().Printf("%i:%i", a, b);
                        a = c;
                        pajazzo = 1;
                    }
                    else if (c >= b)
                    {
                        b = c;
                    }
                }
                else
                {  // pajazzo == 0
                    a = c;
                    pajazzo = 1;
                }
                if (i == epsg_codes.size())
                {
                    // must empty the pajazzo before leaving
                    if (codes != "")
                    {
                        codes += ",";
                    }
                    if (pajazzo == 1)
                    {
                        codes += CPLString().Printf("%i", a);
                    }
                    else if (pajazzo == 2)
                    {
                        codes += CPLString().Printf("%i:%i", a, b);
                    }
                    break;
                }
                ++i;
            }
            if (words != "")
            {
                words += ",";
            }
            words += "EPSG:" + codes;
        }
    }
    return words;
}

std::string ParseCRS(CPLXMLNode *node)
{
    // test for attrs crs (OWS) and srsName (GML), and text contents of subnode
    // (GridBaseCRS)
    std::string crs = CPLGetXMLValue(node, "crs", "");
    if (crs == "")
    {
        crs = CPLGetXMLValue(node, "srsName", "");
        if (crs == "")
        {
            crs = CPLGetXMLValue(node, "GridBaseCRS", "");
        }
    }
    if (crs == "")
    {
        return crs;
    }
    // split compound names
    // see for example
    // http://www.eurogeographics.org/sites/default/files/2016-01-18_INSPIRE-KEN-CovFaq.pdf
    size_t pos = crs.find("?");
    if (pos != std::string::npos)
    {
        if (crs.find("crs-compound?") != std::string::npos)
        {  // 1=uri&2=uri...
            // assuming the first is for X,Y
            crs = crs.substr(pos + 1);
            pos = crs.find("&");
            if (pos != std::string::npos)
            {
                pos = pos - 2;
            }
            crs = crs.substr(2, pos);
        }
    }
    return crs;
}

// if appropriate, try to create WKT description from CRS name
// return false if failure
// appropriate means, that the name is a real CRS
bool CRS2Projection(const std::string &crs, OGRSpatialReference *sr,
                    char **projection)
{
    if (*projection != nullptr)
    {
        CPLFree(*projection);
    }
    *projection = nullptr;
    if (crs.empty())
    {
        return true;
    }
    if (crs.find(":imageCRS") != std::string::npos ||
        crs.find("/Index1D") != std::string::npos ||
        crs.find("/Index2D") != std::string::npos ||
        crs.find("/Index3D") != std::string::npos ||
        crs.find("/AnsiDate") != std::string::npos)
    {
        // not a map projection
        return true;
    }
    std::string crs2 = crs;
    // rasdaman uses urls, which return gml:ProjectedCRS XML, which is not
    // recognized by GDAL currently
    if (crs2.find("EPSG") != std::string::npos)
    {  // ...EPSG...(\d+)
        size_t pos1 = crs2.find_last_of(DIGITS);
        if (pos1 != std::string::npos)
        {
            size_t pos2 = pos1 - 1;
            char c = crs2.at(pos2);
            while (strchr(DIGITS, c))
            {
                pos2 = pos2 - 1;
                c = crs2.at(pos2);
            }
            crs2 = "EPSGA:" + crs2.substr(pos2 + 1, pos1 - pos2);
        }
    }
    OGRSpatialReference local_sr;
    OGRSpatialReference *sr_pointer = sr != nullptr ? sr : &local_sr;
    if (sr_pointer->SetFromUserInput(
            crs2.c_str(),
            OGRSpatialReference::SET_FROM_USER_INPUT_LIMITATIONS_get()) ==
        OGRERR_NONE)
    {
        sr_pointer->exportToWkt(projection);
        return true;
    }
    return false;
}

bool CRSImpliesAxisOrderSwap(const std::string &crs, bool &swap,
                             char **projection)
{
    OGRSpatialReference oSRS;
    char *tmp = nullptr;
    swap = false;
    if (!CRS2Projection(crs, &oSRS, &tmp))
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "Unable to interpret coverage CRS '%s'.", crs.c_str());
        CPLFree(tmp);
        return false;
    }
    if (tmp)
    {
        if (projection != nullptr)
        {
            *projection = tmp;
        }
        else
        {
            CPLFree(tmp);
        }
        swap = oSRS.EPSGTreatsAsLatLong() || oSRS.EPSGTreatsAsNorthingEasting();
    }
    return true;
}

std::vector<std::vector<int>> ParseGridEnvelope(CPLXMLNode *node,
                                                bool swap_the_first_two)
{
    std::vector<std::vector<int>> envelope;
    std::vector<std::string> array =
        Split(CPLGetXMLValue(node, "low", ""), " ", swap_the_first_two);
    std::vector<int> lows;
    for (unsigned int i = 0; i < array.size(); ++i)
    {
        lows.push_back(atoi(array[i].c_str()));
    }
    envelope.push_back(std::move(lows));
    array = Split(CPLGetXMLValue(node, "high", ""), " ", swap_the_first_two);
    std::vector<int> highs;
    for (unsigned int i = 0; i < array.size(); ++i)
    {
        highs.push_back(atoi(array[i].c_str()));
    }
    envelope.push_back(std::move(highs));
    return envelope;
}

std::vector<std::string> ParseBoundingBox(CPLXMLNode *node)
{
    std::vector<std::string> bbox;
    std::string lc = CPLGetXMLValue(node, "lowerCorner", ""), uc;
    if (lc == "")
    {
        lc = CPLGetXMLValue(node, "LowerCorner", "");
    }
    if (lc == "")
    {
        for (CPLXMLNode *n = node->psChild; n != nullptr; n = n->psNext)
        {
            if (n->eType != CXT_Element || !EQUAL(n->pszValue, "pos"))
            {
                continue;
            }
            if (lc == "")
            {
                lc = CPLGetXMLValue(node, nullptr, "");
            }
            else
            {
                uc = CPLGetXMLValue(node, nullptr, "");
            }
        }
    }
    else
    {
        uc = CPLGetXMLValue(node, "upperCorner", "");
        if (uc == "")
        {
            uc = CPLGetXMLValue(node, "UpperCorner", "");
        }
    }
    if (lc != "" && uc != "")
    {
        bbox.push_back(lc);
        bbox.push_back(uc);
    }
    // time extent if node is an EnvelopeWithTimePeriod
    lc = CPLGetXMLValue(node, "beginPosition", "");
    if (lc != "")
    {
        uc = CPLGetXMLValue(node, "endPosition", "");
        bbox.push_back(lc + "," + uc);
    }
    return bbox;
}

}  // namespace WCSUtils
